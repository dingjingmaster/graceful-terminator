#include "gt-config.h"

#include <glib/gi18n.h>

#include "gt-tab.h"
#include "gt-pages.h"
#include "gt-settings.h"
#include "gt-marshals.h"
#include "gt-close-dialog.h"


typedef struct _GtPagesPrivate GtPagesPrivate;
struct _GtPagesPrivate
{
  GtkWidget            *view;
  char                 *title;
  GFile                *path;
  GtTab               *active_page;
  gboolean              is_active;
  GtStatus             page_status;
  gboolean              search_mode_enabled;

  GtkWidget            *status;
  GtkWidget            *status_revealer;

  int                   last_cols;
  int                   last_rows;
  guint                 timeout;

  GSignalGroup         *active_page_signals;
  GBindingGroup        *active_page_binds;

  GBinding             *is_active_bind;

  AdwTabPage           *action_page;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtPages, gt_pages, ADW_TYPE_BIN)


enum
{
    PROP_0,
    PROP_TAB_VIEW,
    PROP_TAB_COUNT,
    PROP_TITLE,
    PROP_PATH,
    PROP_ACTIVE_PAGE,
    PROP_IS_ACTIVE,
    PROP_STATUS,
    PROP_SEARCH_MODE_ENABLED,
    LAST_PROP
};
static GParamSpec *pspecs[LAST_PROP] = { NULL, };


enum
{
    ZOOM,
    CREATE_TEAROFF_HOST,
    N_SIGNALS
};
static guint signals[N_SIGNALS];

static void gt_pages_dispose (GObject *object)
{
    GtPages *self = GT_PAGES (object);
    GtPagesPrivate *priv = gt_pages_get_instance_private (self);

    g_clear_handle_id (&priv->timeout, g_source_remove);

    g_clear_pointer (&priv->title, g_free);
    g_clear_object (&priv->path);

    g_clear_weak_pointer (&priv->active_page);

    G_OBJECT_CLASS (gt_pages_parent_class)->dispose (object);
}


static void gt_pages_get_property (GObject* object, guint property_id, GValue* value, GParamSpec* pspec)
{
    GtPages *self = GT_PAGES (object);
    GtPagesPrivate *priv = gt_pages_get_instance_private (self);

    switch (property_id) {
        case PROP_TAB_COUNT:
            g_value_set_uint (value, adw_tab_view_get_n_pages (ADW_TAB_VIEW (priv->view)));
            break;
        case PROP_TAB_VIEW:
            g_value_set_object (value, priv->view);
            break;
        case PROP_TITLE:
            g_value_set_string (value, priv->title);
            break;
        case PROP_PATH:
            g_value_set_object (value, priv->path);
            break;
        case PROP_ACTIVE_PAGE:
            g_value_set_object (value, priv->active_page);
            break;
        case PROP_IS_ACTIVE:
            g_value_set_boolean (value, priv->is_active);
            break;
        case PROP_STATUS:
            g_value_set_flags (value, priv->page_status);
            break;
        case PROP_SEARCH_MODE_ENABLED:
            g_value_set_boolean (value, priv->search_mode_enabled);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static void gt_pages_set_property (GObject* object, guint property_id, const GValue* value, GParamSpec* pspec)
{
    GtPages *self = GT_PAGES (object);
    GtPagesPrivate *priv = gt_pages_get_instance_private (self);

    switch (property_id) {
        case PROP_TITLE:
            g_clear_pointer (&priv->title, g_free);
            priv->title = g_value_dup_string (value);
            break;
        case PROP_PATH:
            g_set_object (&priv->path, g_value_get_object (value));
            break;
        case PROP_ACTIVE_PAGE:
            if (priv->active_page) {
                g_object_set (priv->active_page, "is-active", FALSE, NULL);
            }
            g_clear_object (&priv->is_active_bind);
            g_set_weak_pointer (&priv->active_page, g_value_get_object (value));
            if (priv->active_page) {
                priv->is_active_bind = g_object_bind_property (self, "is-active", priv->active_page, "is-active", G_BINDING_SYNC_CREATE);
            }
            break;
        case PROP_IS_ACTIVE:
            priv->is_active = g_value_get_boolean (value);
            break;
        case PROP_STATUS:
            priv->page_status = g_value_get_flags (value);
            break;
        case PROP_SEARCH_MODE_ENABLED:
            priv->search_mode_enabled = g_value_get_boolean (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static gboolean size_timeout (GtPages *self)
{
    GtPagesPrivate *priv = gt_pages_get_instance_private (self);

    priv->timeout = 0;

    gtk_revealer_set_reveal_child (GTK_REVEALER (priv->status_revealer), FALSE);

    return G_SOURCE_REMOVE;
}


static void size_changed (GtTab* tab, guint rows, guint cols, GtPages* self)
{
    GtPagesPrivate *priv = gt_pages_get_instance_private (self);
    g_autofree char *label = NULL;

    if (gtk_widget_in_destruction (GTK_WIDGET (self)))
        return;

    if (cols == priv->last_cols && rows == priv->last_rows) {
        return;
    }

    priv->last_cols = cols;
    priv->last_rows = rows;

    if (gtk_window_is_maximized (GTK_WINDOW (gtk_widget_get_root (GTK_WIDGET (self))))) {
        return;
    }

    g_clear_handle_id (&priv->timeout, g_source_remove);
    priv->timeout = g_timeout_add (800, G_SOURCE_FUNC (size_timeout), self);
    g_source_set_name_by_id (priv->timeout, "[kgx] resize label timeout");

    label = g_strdup_printf ("%i × %i", cols, rows);

    gtk_label_set_label (GTK_LABEL (priv->status), label);
    gtk_widget_show (priv->status_revealer);
    gtk_revealer_set_reveal_child (GTK_REVEALER (priv->status_revealer), TRUE);
}


static void died (GtTab* page, GtkMessageType type, const char* message, gboolean success, GtPages* self)
{
    GtPagesPrivate *priv;
    AdwTabPage *tab_page;
    gboolean close_on_quit;
    int tab_count;

    priv = gt_pages_get_instance_private (self);
    tab_page = adw_tab_view_get_page (ADW_TAB_VIEW (priv->view), GTK_WIDGET (page));

    g_object_get (page, "close-on-quit", &close_on_quit, NULL);

    if (!(close_on_quit && success)) {
        return;
    }

    g_object_get (self, "tab-count", &tab_count, NULL);

    if (tab_count < 1) {
        return;
    }

    adw_tab_view_close_page (ADW_TAB_VIEW (priv->view), tab_page);
}


static void zoom (GtTab* tab, GtZoom dir, GtPages* self)
{
    g_signal_emit (self, signals[ZOOM], 0, dir);
}


static void page_attached (AdwTabView* view, AdwTabPage* page, int position, GtPages* self)
{
    GtTab *tab;

    g_return_if_fail (ADW_IS_TAB_PAGE (page));

    tab = GT_TAB (adw_tab_page_get_child (page));

    g_object_connect (tab, "signal::died", G_CALLBACK (died), self, "signal::zoom", G_CALLBACK (zoom), self, NULL);
}


static void page_detached (AdwTabView* view, AdwTabPage* page, int position, GtPages* self)
{
    GtPagesPrivate *priv;
    GtkRoot *root;

    g_return_if_fail (ADW_IS_TAB_PAGE (page));

    priv = gt_pages_get_instance_private (self);

    if (adw_tab_view_get_n_pages (ADW_TAB_VIEW (priv->view)) == 0) {
        root = gtk_widget_get_root (GTK_WIDGET (self));

        if (GTK_IS_WINDOW (root)) {
            gtk_window_close (GTK_WINDOW (root));
        }
    }
}


static AdwTabView* create_window (AdwTabView* view, GtPages* self)
{
    GtPages *newPages;
    GtPagesPrivate *priv;

    g_signal_emit (self, signals[CREATE_TEAROFF_HOST], 0, &newPages);

    priv = gt_pages_get_instance_private (newPages);

    return ADW_TAB_VIEW (priv->view);
}


static void close_response (AdwTabPage* page, const char* response)
{
    GtTab *tab = GT_TAB (adw_tab_page_get_child (page));
    GtPages *self = gt_tab_get_pages (tab);
    GtPagesPrivate *priv = gt_pages_get_instance_private (self);

    adw_tab_view_close_page_finish (ADW_TAB_VIEW (priv->view), page, !g_strcmp0 (response, "close"));
}


static gboolean close_page (AdwTabView* view, AdwTabPage* page, GtPages* self)
{
    GtkWidget *dlg;
    g_autoptr (GPtrArray) children = NULL;
    GtkRoot *root;

    children = gt_tab_get_children (GT_TAB (adw_tab_page_get_child (page)));

    if (children->len < 1) {
        return FALSE; // Aka no, I don’t want to block closing
    }

    root = gtk_widget_get_root (GTK_WIDGET (self));

    dlg = gt_close_dialog_new (GT_CONTEXT_TAB, children);

    gtk_window_set_transient_for (GTK_WINDOW (dlg), GTK_WINDOW (root));

    g_signal_connect_swapped (dlg, "response", G_CALLBACK (close_response), page);

    gtk_widget_show (dlg);

    return TRUE; // Block the close
}


static void setup_menu (AdwTabView* view, AdwTabPage* page, GtPages* self)
{
    GtPagesPrivate *priv = gt_pages_get_instance_private (self);

    priv->action_page = page;
}


static void check_revealer (GtkRevealer* revealer, GParamSpec* pspec, GtPages* self)
{
    if (!gtk_revealer_get_child_revealed (revealer))
        gtk_widget_hide (GTK_WIDGET (revealer));
}


static gboolean status_to_icon (GBinding* binding, const GValue* fromValue, GValue* toValue, gpointer udata)
{
    GtStatus status = g_value_get_flags (fromValue);

    if (status & GT_REMOTE)
        g_value_take_object (toValue, g_themed_icon_new ("status-remote-symbolic"));
    else if (status & GT_PRIVILEGED)
        g_value_take_object (toValue, g_themed_icon_new ("status-privileged-symbolic"));
    else
        g_value_set_object (toValue, NULL);

    return TRUE;
}


static gboolean object_accumulator (GSignalInvocationHint* ihint, GValue* returnValue, const GValue* signalValue, gpointer data)
{
    GObject *object = g_value_get_object (signalValue);

    g_value_set_object (returnValue, object);

    return !object;
}


static void gt_pages_class_init (GtPagesClass *klass)
{
    GObjectClass   *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->dispose = gt_pages_dispose;
    object_class->get_property = gt_pages_get_property;
    object_class->set_property = gt_pages_set_property;

    pspecs[PROP_TAB_VIEW] = g_param_spec_object ("tab-view", "Tab View", "The tab view", ADW_TYPE_TAB_VIEW, G_PARAM_READABLE);
    pspecs[PROP_TAB_COUNT] = g_param_spec_uint ("tab-count", "Page Count", "Number of pages", 0, G_MAXUINT32, 0, G_PARAM_READABLE);
    pspecs[PROP_TITLE] = g_param_spec_string ("title", "Title", "The title of the active page", NULL, G_PARAM_READWRITE);
    pspecs[PROP_PATH] = g_param_spec_object ("path", "Path", "The path of the active page", G_TYPE_FILE, G_PARAM_READWRITE);
    pspecs[PROP_ACTIVE_PAGE] = g_param_spec_object ("active-page", NULL, NULL, GT_TYPE_TAB, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    pspecs[PROP_IS_ACTIVE] = g_param_spec_boolean ("is-active", "Is Active", "Is active pages", FALSE, G_PARAM_READWRITE);
    pspecs[PROP_STATUS] = g_param_spec_flags ("status", "Status", "Active page status", GT_TYPE_STATUS, GT_NONE, G_PARAM_READWRITE);
    pspecs[PROP_SEARCH_MODE_ENABLED] = g_param_spec_boolean ("search-mode-enabled", "Search mode enabled", "Whether the search mode is enabled for active page", FALSE, G_PARAM_READWRITE);

    g_object_class_install_properties (object_class, LAST_PROP, pspecs);

    signals[ZOOM] = g_signal_new ("zoom", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL, gt_marshals_VOID__ENUM, G_TYPE_NONE, 1, GT_TYPE_ZOOM);
    signals[CREATE_TEAROFF_HOST] = g_signal_new ("create-tearoff-host", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, object_accumulator, NULL, gt_marshals_OBJECT__VOID, GT_TYPE_PAGES, 0);

    gtk_widget_class_set_template_from_resource (widget_class, GT_APPLICATION_PATH "gt-pages.ui");

    gtk_widget_class_bind_template_child_private (widget_class, GtPages, view);
    gtk_widget_class_bind_template_child_private (widget_class, GtPages, status);
    gtk_widget_class_bind_template_child_private (widget_class, GtPages, status_revealer);
    gtk_widget_class_bind_template_child_private (widget_class, GtPages, active_page_signals);
    gtk_widget_class_bind_template_child_private (widget_class, GtPages, active_page_binds);

    gtk_widget_class_bind_template_callback (widget_class, page_attached);
    gtk_widget_class_bind_template_callback (widget_class, page_detached);
    gtk_widget_class_bind_template_callback (widget_class, create_window);
    gtk_widget_class_bind_template_callback (widget_class, close_page);
    gtk_widget_class_bind_template_callback (widget_class, setup_menu);
    gtk_widget_class_bind_template_callback (widget_class, check_revealer);

    gtk_widget_class_set_css_name (widget_class, "pages");
}


static void gt_pages_init (GtPages* self)
{
    GtPagesPrivate* priv = gt_pages_get_instance_private (self);

    gtk_widget_init_template (GTK_WIDGET (self));

    g_signal_group_connect (priv->active_page_signals, "size-changed", G_CALLBACK (size_changed), self);
    g_binding_group_bind (priv->active_page_binds, "search-mode-enabled", self, "search-mode-enabled", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

    adw_tab_view_remove_shortcuts (ADW_TAB_VIEW (priv->view), ADW_TAB_VIEW_SHORTCUT_CONTROL_HOME | ADW_TAB_VIEW_SHORTCUT_CONTROL_END | ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_UP | ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_DOWN | ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_HOME | ADW_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_END);
}


void gt_pages_add_page (GtPages* self, GtTab* tab)
{
    GtPagesPrivate* priv;
    AdwTabPage* page;

    g_return_if_fail (GT_IS_PAGES (self));

    priv = gt_pages_get_instance_private (self);

    gt_tab_set_initial_title (tab, priv->title, priv->path);

    page = adw_tab_view_add_page (ADW_TAB_VIEW (priv->view), GTK_WIDGET (tab), NULL);
    g_object_bind_property (tab, "tab-title", page, "title", G_BINDING_SYNC_CREATE);
    g_object_bind_property (tab, "tab-tooltip", page, "tooltip", G_BINDING_SYNC_CREATE);
    g_object_bind_property (tab, "needs-attention", page, "needs-attention", G_BINDING_SYNC_CREATE);
    g_object_bind_property_full (tab, "tab-status", page, "icon", G_BINDING_SYNC_CREATE, status_to_icon, NULL, NULL, NULL);
}


void gt_pages_remove_page (GtPages* self, GtTab* page)
{
    GtPagesPrivate *priv;
    AdwTabPage *tab_page;

    g_return_if_fail (GT_IS_PAGES (self));

    priv = gt_pages_get_instance_private (self);

    if (!page) {
        tab_page = adw_tab_view_get_selected_page (ADW_TAB_VIEW (priv->view));
        adw_tab_view_close_page (ADW_TAB_VIEW (priv->view), tab_page);
        return;
    }

    g_return_if_fail (GT_IS_TAB (page));

    tab_page = adw_tab_view_get_page (ADW_TAB_VIEW (priv->view), GTK_WIDGET (page));
    adw_tab_view_close_page (ADW_TAB_VIEW (priv->view), tab_page);
}

void gt_pages_focus_page (GtPages* self, GtTab* page)
{
    GtPagesPrivate *priv;
    AdwTabPage *index;

    g_return_if_fail (GT_IS_PAGES (self));
    g_return_if_fail (GT_IS_TAB (page));

    priv = gt_pages_get_instance_private (self);

    index = adw_tab_view_get_page (ADW_TAB_VIEW (priv->view), GTK_WIDGET (page));

    g_return_if_fail (index != NULL);

    adw_tab_view_set_selected_page (ADW_TAB_VIEW (priv->view), index);

    gtk_widget_grab_focus (GTK_WIDGET (page));
}

GtStatus gt_pages_current_status (GtPages *self)
{
    GtPagesPrivate *priv;

    g_return_val_if_fail (GT_IS_PAGES (self), GT_NONE);

    priv = gt_pages_get_instance_private (self);

    return priv->page_status;
}

int gt_pages_count (GtPages *self)
{
    GtPagesPrivate *priv;

    g_return_val_if_fail (GT_IS_PAGES (self), GT_NONE);

    priv = gt_pages_get_instance_private (self);

    return adw_tab_view_get_n_pages (ADW_TAB_VIEW (priv->view));
}

GPtrArray* gt_pages_get_children (GtPages *self)
{
    GtPagesPrivate *priv;
    GPtrArray *children;
    guint n;

    g_return_val_if_fail (GT_IS_PAGES (self), NULL);

    priv = gt_pages_get_instance_private (self);

    children = g_ptr_array_new_full (10, (GDestroyNotify) gt_process_unref);

    n = adw_tab_view_get_n_pages (ADW_TAB_VIEW (priv->view));

    for (guint i = 0; i < n; i++) {
        AdwTabPage *page = adw_tab_view_get_nth_page (ADW_TAB_VIEW (priv->view), i);
        g_autoptr (GPtrArray) page_children = NULL;

        page_children = gt_tab_get_children (GT_TAB (adw_tab_page_get_child (page)));

        for (int j = 0; j < page_children->len; j++) {
            g_ptr_array_add (children, g_ptr_array_steal_index (page_children, j));
        }
    }

    return children;
}


void gt_pages_close_page (GtPages *self)
{
    GtPagesPrivate *priv;
    AdwTabPage *page;

    g_return_if_fail (GT_IS_PAGES (self));

    priv = gt_pages_get_instance_private (self);
    page = priv->action_page;

    if (!page)
        page = adw_tab_view_get_selected_page (ADW_TAB_VIEW (priv->view));

    adw_tab_view_close_page (ADW_TAB_VIEW (priv->view), page);
}

void gt_pages_detach_page (GtPages *self)
{
    GtPagesPrivate *priv;
    AdwTabPage *page;
    AdwTabView *new_view;

    g_return_if_fail (GT_IS_PAGES (self));

    priv = gt_pages_get_instance_private (self);
    page = priv->action_page;

    if (!page)
        page = adw_tab_view_get_selected_page (ADW_TAB_VIEW (priv->view));

    new_view = create_window (ADW_TAB_VIEW (priv->view), self);
    adw_tab_view_transfer_page (ADW_TAB_VIEW (priv->view), page, new_view, 0);
}
