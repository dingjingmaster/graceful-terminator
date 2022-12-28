#include "gt-tab.h"


#include <glib/gi18n.h>

#define PCRE2_CODE_UNIT_WIDTH 0

#include <pcre2.h>

#include "gt-util.h"
#include "gt-pages.h"
#include "gt-config.h"
#include "gt-settings.h"
#include "gt-terminal.h"
#include "gt-marshals.h"
#include "gt-application.h"


typedef struct _GtTabPrivate GtTabPrivate;
struct _GtTabPrivate
{
    guint id;

    GtApplication *application;
    GtSettings *settings;

    char *title;
    char *tooltip;
    GFile *path;
    GtStatus status;

    gboolean is_active;
    gboolean close_on_quit;
    gboolean needs_attention;
    gboolean search_mode_enabled;

    GtTerminal *terminal;
    GSignalGroup *terminal_signals;
    GBindingGroup *terminal_binds;

    GtkWidget *stack;
    GtkWidget *spinner_revealer;
    GtkWidget *content;
    guint spinner_timeout;

    GtkWidget *revealer;
    GtkWidget *label;
    GtkWidget *search_entry;
    GtkWidget *search_bar;
    char *last_search;

    /* Remote/root states */
    GHashTable *root;
    GHashTable *remote;
    GHashTable *children;

    char *notification_id;
};


static void gt_tab_buildable_iface_init(GtkBuildableIface *iface);

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (GtTab, gt_tab, GTK_TYPE_BOX, G_ADD_PRIVATE (GtTab)
    G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, gt_tab_buildable_iface_init))


enum
{
    PROP_0,
    PROP_APPLICATION,
    PROP_SETTINGS,
    PROP_TERMINAL,
    PROP_TAB_TITLE,
    PROP_TAB_PATH,
    PROP_TAB_STATUS,
    PROP_TAB_TOOLTIP,
    PROP_IS_ACTIVE,
    PROP_CLOSE_ON_QUIT,
    PROP_NEEDS_ATTENTION,
    PROP_SEARCH_MODE_ENABLED,
    LAST_PROP
};
static GParamSpec *pspecs[LAST_PROP] = {NULL,};


enum
{
    SIZE_CHANGED, ZOOM, DIED, N_SIGNALS
};
static guint signals[N_SIGNALS];


static void gt_tab_dispose(GObject *object)
{
    GtTab *self = GT_TAB (object);
    GtTabPrivate *priv = gt_tab_get_instance_private (self);

    g_clear_handle_id (&priv->spinner_timeout, g_source_remove);

    if (priv->notification_id) {
        g_application_withdraw_notification (G_APPLICATION (priv->application), priv->notification_id);
        g_clear_pointer (&priv->notification_id, g_free);
    }

    g_clear_object (&priv->application);
    g_clear_object (&priv->settings);
    g_clear_object (&priv->terminal);

    g_clear_pointer (&priv->title, g_free);
    g_clear_pointer (&priv->tooltip, g_free);
    g_clear_object (&priv->path);

    g_clear_pointer (&priv->root, g_hash_table_unref);
    g_clear_pointer (&priv->remote, g_hash_table_unref);
    g_clear_pointer (&priv->children, g_hash_table_unref);

    g_clear_pointer (&priv->last_search, g_free);

    G_OBJECT_CLASS (gt_tab_parent_class)->dispose (object);
}


static void search_enabled(GObject *object, GParamSpec *pspec, GtTab *self)
{
    GtTabPrivate *priv = gt_tab_get_instance_private (self);

    if (!gtk_search_bar_get_search_mode (GTK_SEARCH_BAR (priv->search_bar))) {
        gtk_widget_grab_focus (GTK_WIDGET (self));
    }
}


static void search_changed(GtkSearchBar *bar, GtTab *self)
{
    GtTabPrivate *priv = gt_tab_get_instance_private (self);
    const char *search = NULL;
    VteRegex *regex;
    g_autoptr (GError) error = NULL;
    gboolean narrowing_down;
    guint32 flags = PCRE2_MULTILINE;

    search = gtk_editable_get_text (GTK_EDITABLE (priv->search_entry));

    if (search) {
        g_autofree char *lowercase = g_utf8_strdown (search, -1);

        if (!g_strcmp0 (lowercase, search))
            flags |= PCRE2_CASELESS;
    }

    regex = vte_regex_new_for_search (g_regex_escape_string (search, -1), -1, flags, &error);

    if (error) {
        g_warning ("Search error: %s", error->message);
        return;
    }

    narrowing_down = search && priv->last_search && g_strrstr (priv->last_search, search);

    g_clear_pointer (&priv->last_search, g_free);
    priv->last_search = g_strdup (search);

    if (!narrowing_down)
        vte_terminal_search_find_previous (VTE_TERMINAL (priv->terminal));

    vte_terminal_search_set_regex (VTE_TERMINAL (priv->terminal), regex, 0);

    if (narrowing_down)
        vte_terminal_search_find_previous (VTE_TERMINAL (priv->terminal));

    vte_terminal_search_find_next (VTE_TERMINAL (priv->terminal));
}


static void search_next(GtkSearchBar *bar, GtTab *self)
{
    GtTabPrivate *priv = gt_tab_get_instance_private (self);

    vte_terminal_search_find_next (VTE_TERMINAL (priv->terminal));
}


static void search_prev(GtkSearchBar *bar, GtTab *self)
{
    GtTabPrivate *priv = gt_tab_get_instance_private (self);

    vte_terminal_search_find_previous (VTE_TERMINAL (priv->terminal));
}


static void spinner_mapped(GtkSpinner *spinner, GtTab *self)
{
    gtk_spinner_start (spinner);
}


static void spinner_unmapped(GtkSpinner *spinner, GtTab *self)
{
    gtk_spinner_stop (spinner);
}


static gboolean start_spinner_timeout_cb(GtTab *self)
{
    GtTabPrivate *priv = gt_tab_get_instance_private (self);

    gtk_revealer_set_reveal_child (GTK_REVEALER (priv->spinner_revealer), TRUE);
    priv->spinner_timeout = 0;

    return G_SOURCE_REMOVE;
}


static void gt_tab_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
    GtTab *self = GT_TAB (object);
    GtTabPrivate *priv = gt_tab_get_instance_private (self);

    switch (property_id) {
        case PROP_APPLICATION:
            g_value_set_object (value, priv->application);
            break;
        case PROP_SETTINGS:
            g_value_set_object (value, priv->settings);
            break;
        case PROP_TERMINAL:
            g_value_set_object (value, priv->terminal);
            break;
        case PROP_TAB_TITLE:
            g_value_set_string (value, priv->title);
            break;
        case PROP_TAB_PATH:
            g_value_set_object (value, priv->path);
            break;
        case PROP_TAB_STATUS:
            g_value_set_flags (value, priv->status);
            break;
        case PROP_TAB_TOOLTIP:
            g_value_set_string (value, priv->tooltip);
            break;
        case PROP_IS_ACTIVE:
            g_value_set_boolean (value, priv->is_active);
            break;
        case PROP_CLOSE_ON_QUIT:
            g_value_set_boolean (value, priv->close_on_quit);
            break;
        case PROP_NEEDS_ATTENTION:
            g_value_set_boolean (value, priv->needs_attention);
            break;
        case PROP_SEARCH_MODE_ENABLED:
            g_value_set_boolean (value, priv->search_mode_enabled);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static void gt_tab_set_is_active(GtTab *self, gboolean active)
{
    GtTabPrivate *priv;

    g_return_if_fail (GT_IS_TAB (self));

    priv = gt_tab_get_instance_private (self);

    if (active == priv->is_active) {
        return;
    }

    priv->is_active = active;

    if (!active && priv->notification_id) {
        g_application_withdraw_notification (G_APPLICATION (priv->application), priv->notification_id);
        g_clear_pointer (&priv->notification_id, g_free);
    }
    g_object_set (self, "needs-attention", FALSE, NULL);

    g_object_notify_by_pspec (G_OBJECT (self), pspecs[PROP_IS_ACTIVE]);
}


static void gt_tab_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    GtTab *self = GT_TAB (object);
    GtTabPrivate *priv = gt_tab_get_instance_private (self);

    switch (property_id) {
        case PROP_APPLICATION:
            if (priv->application) {
                g_critical ("Application was already set %p", priv->application);
            }
            priv->application = g_value_dup_object (value);
            gt_application_add_page (priv->application, self);
            break;
        case PROP_SETTINGS:
            g_set_object (&priv->settings, g_value_get_object (value));
            break;
        case PROP_TERMINAL:
            g_set_object (&priv->terminal, g_value_get_object (value));
            break;
        case PROP_TAB_TITLE:
            g_clear_pointer (&priv->title, g_free);
            priv->title = g_value_dup_string (value);
            break;
        case PROP_TAB_PATH:
            g_set_object (&priv->path, g_value_get_object (value));
            break;
        case PROP_TAB_STATUS:
            priv->status = g_value_get_flags (value);
            break;
        case PROP_TAB_TOOLTIP:
            g_clear_pointer (&priv->tooltip, g_free);
            priv->tooltip = g_value_dup_string (value);
            break;
        case PROP_IS_ACTIVE:
            gt_tab_set_is_active (self, g_value_get_boolean (value));
            break;
        case PROP_CLOSE_ON_QUIT:
            priv->close_on_quit = g_value_get_boolean (value);
            break;
        case PROP_NEEDS_ATTENTION:
            priv->needs_attention = g_value_get_boolean (value);
            break;
        case PROP_SEARCH_MODE_ENABLED:
            priv->search_mode_enabled = g_value_get_boolean (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static gboolean drop(GtkDropTarget *target, const GValue *value, GtTab *self)
{
    gt_tab_accept_drop (self, value);

    return TRUE;
}


static gboolean gt_tab_grab_focus(GtkWidget *widget)
{
    GtTab *self = GT_TAB (widget);
    GtTabPrivate *priv = gt_tab_get_instance_private (self);

    if (priv->terminal) {
        gtk_widget_grab_focus (GTK_WIDGET (priv->terminal));
        return GDK_EVENT_STOP;
    }

    return GTK_WIDGET_CLASS (gt_tab_parent_class)->grab_focus (widget);
}


static void gt_tab_real_start(GtTab *tab, GAsyncReadyCallback callback, gpointer callback_data)
{
    g_critical ("%s doesn't implement start", G_OBJECT_TYPE_NAME (tab));
}


static GPid gt_tab_real_start_finish(GtTab *tab, GAsyncResult *res, GError **error)
{
    g_critical ("%s doesn't implement start_finish", G_OBJECT_TYPE_NAME (tab));

    return 0;
}


static void gt_tab_real_died(GtTab *self, GtkMessageType type, const char *message, gboolean success)
{
    GtTabPrivate *priv;

    g_return_if_fail (GT_IS_TAB (self));

    priv = gt_tab_get_instance_private (self);

    gtk_label_set_markup (GTK_LABEL (priv->label), message);

    if (type == GTK_MESSAGE_ERROR) {
        gtk_widget_add_css_class (priv->revealer, "error");
    } else {
        gtk_widget_remove_css_class (priv->revealer, "error");
    }

    gtk_revealer_set_reveal_child (GTK_REVEALER (priv->revealer), TRUE);
}


static void gt_tab_class_init(GtTabClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS   (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtTabClass *tab_class = GT_TAB_CLASS (klass);

    object_class->dispose = gt_tab_dispose;
    object_class->get_property = gt_tab_get_property;
    object_class->set_property = gt_tab_set_property;

    widget_class->grab_focus = gt_tab_grab_focus;

    tab_class->start = gt_tab_real_start;
    tab_class->start_finish = gt_tab_real_start_finish;
    tab_class->died = gt_tab_real_died;

    pspecs[PROP_APPLICATION] = g_param_spec_object ("application", "Application", "The application",
                                                    GT_TYPE_APPLICATION, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    pspecs[PROP_SETTINGS] = g_param_spec_object ("settings", NULL, NULL, GT_TYPE_SETTINGS,
                                                 G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    pspecs[PROP_TERMINAL] = g_param_spec_object ("terminal", NULL, NULL, GT_TYPE_TERMINAL,
                                                 G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    pspecs[PROP_TAB_TITLE] = g_param_spec_string ("tab-title", "Page Title", "Title for this tab", NULL,
                                                  G_PARAM_READWRITE);

    pspecs[PROP_TAB_PATH] = g_param_spec_object ("tab-path", "Page Path", "Current path", G_TYPE_FILE,
                                                 G_PARAM_READWRITE);


    pspecs[PROP_TAB_STATUS] = g_param_spec_flags ("tab-status", "Page Status", "Session status", GT_TYPE_STATUS,
                                                  GT_NONE, G_PARAM_READWRITE);

    pspecs[PROP_TAB_TOOLTIP] = g_param_spec_string ("tab-tooltip", "Tab Tooltip",
                                                    "Extra information to show in the tooltip", NULL,
                                                    G_PARAM_READWRITE);


    pspecs[PROP_IS_ACTIVE] = g_param_spec_boolean ("is-active", "Is Active", "Current tab", FALSE,
                                                   G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

    pspecs[PROP_CLOSE_ON_QUIT] = g_param_spec_boolean ("close-on-quit", "Close on quit",
                                                       "Should the tab close when dead", FALSE, G_PARAM_READWRITE);

    pspecs[PROP_NEEDS_ATTENTION] = g_param_spec_boolean ("needs-attention", "Needs attention",
                                                         "Whether the tab needs attention", FALSE, G_PARAM_READWRITE);

    pspecs[PROP_SEARCH_MODE_ENABLED] = g_param_spec_boolean ("search-mode-enabled", "Search mode enabled",
                                                             "Whether the search mode is enabled for active page",
                                                             FALSE, G_PARAM_READWRITE);

    g_object_class_install_properties (object_class, LAST_PROP, pspecs);

    signals[SIZE_CHANGED] = g_signal_new ("size-changed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                                          gt_marshals_VOID__UINT_UINT, G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_UINT);

    signals[ZOOM] = g_signal_new ("zoom", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                                  gt_marshals_VOID__ENUM, G_TYPE_NONE, 1, GT_TYPE_ZOOM);

    signals[DIED] = g_signal_new ("died", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
                                  G_STRUCT_OFFSET (GtTabClass, died), NULL, NULL, gt_marshals_VOID__ENUM_STRING_BOOLEAN,
                                  G_TYPE_NONE, 3, GTK_TYPE_MESSAGE_TYPE, G_TYPE_STRING, G_TYPE_BOOLEAN);

    gtk_widget_class_set_template_from_resource (widget_class, GT_APPLICATION_PATH "gt-tab.ui");

    gtk_widget_class_bind_template_child_private (widget_class, GtTab, stack);
    gtk_widget_class_bind_template_child_private (widget_class, GtTab, spinner_revealer);
    gtk_widget_class_bind_template_child_private (widget_class, GtTab, revealer);
    gtk_widget_class_bind_template_child_private (widget_class, GtTab, label);
    gtk_widget_class_bind_template_child_private (widget_class, GtTab, search_entry);
    gtk_widget_class_bind_template_child_private (widget_class, GtTab, search_bar);
    gtk_widget_class_bind_template_child_private (widget_class, GtTab, terminal_signals);
    gtk_widget_class_bind_template_child_private (widget_class, GtTab, terminal_binds);

    gtk_widget_class_bind_template_callback (widget_class, search_enabled);
    gtk_widget_class_bind_template_callback (widget_class, search_changed);
    gtk_widget_class_bind_template_callback (widget_class, search_next);
    gtk_widget_class_bind_template_callback (widget_class, search_prev);
    gtk_widget_class_bind_template_callback (widget_class, spinner_mapped);
    gtk_widget_class_bind_template_callback (widget_class, spinner_unmapped);
}


static void gt_tab_add_child(GtkBuildable *buildable, GtkBuilder *builder, GObject *child, const char *type)
{
    GtTab *self = GT_TAB (buildable);
    GtTabPrivate *priv;

    g_return_if_fail (GT_IS_TAB (self));
    g_return_if_fail (GTK_IS_WIDGET (child));

    priv = gt_tab_get_instance_private (self);

    if (type && g_str_equal (type, "content")) {
        g_set_weak_pointer (&priv->content, GTK_WIDGET (child));
        gtk_stack_add_named (GTK_STACK (priv->stack), GTK_WIDGET (child), "content");
    } else if (GTK_IS_WIDGET (child)) {
        gtk_box_append (GTK_BOX (self), GTK_WIDGET (child));
    }
}


static void gt_tab_buildable_iface_init(GtkBuildableIface *iface)
{
    iface->add_child = gt_tab_add_child;
}


static void size_changed(GtTerminal *term, guint rows, guint cols, GtTab *self)
{
    g_signal_emit (self, signals[SIZE_CHANGED], 0, rows, cols);
}


static void font_increase(GtTerminal *term, GtTab *self)
{
    g_signal_emit (self, signals[ZOOM], 0, GT_ZOOM_IN);
}


static void font_decrease(GtTerminal *term, GtTab *self)
{
    g_signal_emit (self, signals[ZOOM], 0, GT_ZOOM_OUT);
}


static void gt_tab_init(GtTab *self)
{
    static guint last_id = 0;
    GtTabPrivate *priv = gt_tab_get_instance_private (self);
    GtkDropTarget *target;

    last_id++;

    priv->id = last_id;

    priv->root = g_hash_table_new (g_direct_hash, g_direct_equal);
    priv->remote = g_hash_table_new (g_direct_hash, g_direct_equal);
    priv->children = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, (GDestroyNotify) gt_process_unref);

    gtk_widget_init_template (GTK_WIDGET (self));

    g_signal_group_connect (priv->terminal_signals, "size-changed", G_CALLBACK (size_changed),
                            self), g_signal_group_connect (priv->terminal_signals, "increase-font-size",
                                                           G_CALLBACK (font_increase), self), g_signal_group_connect (
        priv->terminal_signals, "decrease-font-size", G_CALLBACK (font_decrease), self),

        g_binding_group_bind (priv->terminal_binds, "window-title", self, "tab-title", G_BINDING_SYNC_CREATE);
    g_binding_group_bind (priv->terminal_binds, "path", self, "tab-path", G_BINDING_SYNC_CREATE);

    gtk_search_bar_connect_entry (GTK_SEARCH_BAR (priv->search_bar), GTK_EDITABLE (priv->search_entry));

    target = gtk_drop_target_new (G_TYPE_STRING, GDK_ACTION_COPY);
    g_signal_connect (target, "drop", G_CALLBACK (drop), self);
    gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (target));
}


void gt_tab_start(GtTab *self, GAsyncReadyCallback callback, gpointer callback_data)
{
    GtTabPrivate *priv;

    g_return_if_fail (GT_IS_TAB (self));
    g_return_if_fail (GT_TAB_GET_CLASS (self)->start);

    priv = gt_tab_get_instance_private (self);

    priv->spinner_timeout = g_timeout_add (100, G_SOURCE_FUNC (start_spinner_timeout_cb), self);

    GT_TAB_GET_CLASS (self)->start (self, callback, callback_data);
}


GPid gt_tab_start_finish(GtTab *self, GAsyncResult *res, GError **error)
{
    GtTabPrivate *priv;
    GPid pid;

    g_return_val_if_fail (GT_IS_TAB (self), 0);
    g_return_val_if_fail (GT_TAB_GET_CLASS (self)->start, 0);

    priv = gt_tab_get_instance_private (self);

    pid = GT_TAB_GET_CLASS (self)->start_finish (self, res, error);

    g_clear_handle_id (&priv->spinner_timeout, g_source_remove);
    gtk_stack_set_visible_child (GTK_STACK (priv->stack), priv->content);
    gtk_widget_grab_focus (GTK_WIDGET (self));

    return pid;
}


void gt_tab_died(GtTab *self, GtkMessageType type, const char *message, gboolean success)
{
    g_signal_emit (self, signals[DIED], 0, type, message, success);
}



GtPages *gt_tab_get_pages(GtTab *self)
{
    GtkWidget *parent;

    parent = gtk_widget_get_ancestor (GTK_WIDGET (self), GT_TYPE_PAGES);

    g_return_val_if_fail (parent, NULL);
    g_return_val_if_fail (GT_IS_PAGES (parent), NULL);

    return GT_PAGES (parent);
}



guint gt_tab_get_id(GtTab *self)
{
    GtTabPrivate *priv;

    g_return_val_if_fail (GT_IS_TAB (self), 0);

    priv = gt_tab_get_instance_private (self);

    return priv->id;
}


static inline GtStatus push_type(GHashTable *table, GPid pid, GtProcess *process, GtkStyleContext *context, GtStatus status)
{
    g_hash_table_insert (table, GINT_TO_POINTER (pid), process != NULL ? g_rc_box_acquire (process) : NULL);

    g_debug ("Now %i %X", g_hash_table_size (table), status);

    return status;
}


void gt_tab_push_child(GtTab *self, GtProcess *process)
{
    GtkStyleContext *context;
    GPid pid = 0;
    GStrv argv;
    g_autofree char *program = NULL;
    GtStatus new_status = GT_NONE;
    GtTabPrivate *priv;

    g_return_if_fail (GT_IS_TAB (self));

    priv = gt_tab_get_instance_private (self);

    context = gtk_widget_get_style_context (GTK_WIDGET (self));
    pid = gt_process_get_pid (process);
    argv = gt_process_get_argv (process);

    if (G_LIKELY (argv[0] != NULL)) {
        program = g_path_get_basename (argv[0]);
    }

    if (G_UNLIKELY (
        g_strcmp0 (program, "ssh") == 0 || g_strcmp0 (program, "mosh-client") == 0 || g_strcmp0 (program, "et") == 0)) {
        new_status |= push_type (priv->remote, pid, NULL, context, GT_REMOTE);
    }

    if (G_UNLIKELY (gt_process_get_is_root (process))) {
        new_status |= push_type (priv->root, pid, NULL, context, GT_PRIVILEGED);
    }

    push_type (priv->children, pid, process, context, GT_NONE);

    if (priv->status != new_status) {
        priv->status = new_status;
        g_object_notify_by_pspec (G_OBJECT (self), pspecs[PROP_TAB_STATUS]);
    }
}


inline static GtStatus pop_type(GHashTable *table, GPid pid, GtkStyleContext *context, GtStatus status)
{
    guint size = 0;

    g_hash_table_remove (table, GINT_TO_POINTER (pid));

    size = g_hash_table_size (table);

    if (G_LIKELY (size <= 0)) {
        g_debug ("No longer %X", status);

        return GT_NONE;
    } else {
        g_debug ("%i %X remaining", size, status);

        return status;
    }
}

void gt_tab_pop_child(GtTab *self, GtProcess *process)
{
    GtkStyleContext *context;
    GPid pid = 0;
    GtStatus new_status = GT_NONE;
    GtTabPrivate *priv;

    g_return_if_fail (GT_IS_TAB (self));

    priv = gt_tab_get_instance_private (self);

    context = gtk_widget_get_style_context (GTK_WIDGET (self));
    pid = gt_process_get_pid (process);

    new_status |= pop_type (priv->remote, pid, context, GT_REMOTE);
    new_status |= pop_type (priv->root, pid, context, GT_PRIVILEGED);
    pop_type (priv->children, pid, context, GT_NONE);

    if (priv->status != new_status) {
        priv->status = new_status;
        g_object_notify_by_pspec (G_OBJECT (self), pspecs[PROP_TAB_STATUS]);
    }

    if (!gt_tab_is_active (self)) {
        g_autoptr (GNotification) noti = NULL;

        noti = g_notification_new (_("Command completed"));
        g_notification_set_body (noti, gt_process_get_exec (process));

        g_notification_set_default_action_and_target (noti, "app.focus-page", "u", priv->id);

        priv->notification_id = g_strdup_printf ("command-completed-%u", priv->id);
        g_application_send_notification (G_APPLICATION (priv->application), priv->notification_id, noti);

        if (!gtk_widget_get_mapped (GTK_WIDGET (self))) {
            g_object_set (self, "needs-attention", TRUE, NULL);
        }
    }
}


gboolean gt_tab_is_active(GtTab *self)
{
    GtTabPrivate *priv;

    g_return_val_if_fail (GT_IS_TAB (self), FALSE);

    priv = gt_tab_get_instance_private (self);

    return priv->is_active;
}


GPtrArray *gt_tab_get_children(GtTab *self)
{
    GtTabPrivate *priv;
    GPtrArray *children;
    GHashTableIter iter;
    gpointer pid, process;

    g_return_val_if_fail (GT_IS_TAB (self), NULL);

    priv = gt_tab_get_instance_private (self);

    children = g_ptr_array_new_full (3, (GDestroyNotify) gt_process_unref);

    g_hash_table_iter_init (&iter, priv->children);
    while (g_hash_table_iter_next (&iter, &pid, &process)) {
        g_ptr_array_add (children, g_rc_box_acquire (process));
    }

    return children;
}


void gt_tab_accept_drop(GtTab *self, const GValue *value)
{
    GtTabPrivate *priv;
    g_autofree char *text = NULL;
    g_auto (GStrv) uris = NULL;

    g_return_if_fail (GT_IS_TAB (self));

    priv = gt_tab_get_instance_private (self);

    uris = g_strsplit (g_value_get_string (value), "\n", 0);

    gt_util_transform_uris_to_quoted_fuse_paths (uris);

    text = gt_util_concat_uris (uris, NULL);

    if (priv->terminal) {
        gt_terminal_accept_paste (KGX_TERMINAL (priv->terminal), text);
    }
}


void gt_tab_set_initial_title(GtTab *self, const char *title, GFile *path)
{
    GtTabPrivate *priv;

    g_return_if_fail (GT_IS_TAB (self));

    priv = gt_tab_get_instance_private (self);

    if (priv->title || priv->path)
        return;

    g_object_set (self, "tab-title", title, "tab-path", path, NULL);
}
