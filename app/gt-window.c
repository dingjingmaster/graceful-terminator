#include "gt-config.h"

#include <glib/gi18n.h>
#include <vte/vte.h>
#include <math.h>
#include <adwaita.h>

#include "rgba.h"

#include "gt-pages.h"
#include "gt-window.h"
#include "gt-watcher.h"
#include "gt-tab-button.h"
#include "gt-application.h"
#include "gt-close-dialog.h"
#include "gt-tab-switcher.h"
#include "gt-theme-switcher.h"

G_DEFINE_TYPE (GtWindow, gt_window, ADW_TYPE_APPLICATION_WINDOW)

enum
{
    PROP_0, PROP_SETTINGS, LAST_PROP
};

static GParamSpec *pspecs[LAST_PROP] = {NULL,};


static void gt_window_dispose(GObject *object)
{
    GtWindow *self = GT_WINDOW (object);

    g_clear_object (&self->settings);
    g_clear_object (&self->tab_actions);

    G_OBJECT_CLASS (gt_window_parent_class)->dispose (object);
}


static void gt_window_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    GtWindow *self = GT_WINDOW (object);

    switch (property_id) {
        case PROP_SETTINGS:
            g_set_object (&self->settings, g_value_get_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static void gt_window_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
    GtWindow *self = GT_WINDOW (object);

    switch (property_id) {
        case PROP_SETTINGS:
            g_value_set_object (value, self->settings);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static void close_response(GtWindow *self)
{
    self->close_anyway = TRUE;

    gtk_window_destroy (GTK_WINDOW (self));
}


static gboolean gt_window_close_request(GtkWindow *window)
{
    GtWindow *self = GT_WINDOW (window);
    GtkWidget *dlg;
    g_autoptr (GPtrArray) children = NULL;

    children = gt_pages_get_children (GT_PAGES (self->pages));

    if (children->len < 1 || self->close_anyway) {
        if (gtk_window_is_active (GTK_WINDOW (self))) {
            int width, height;
            gtk_window_get_default_size (GTK_WINDOW (self), &width, &height);
            gt_settings_set_custom_size (self->settings, width, height);
        }

        return FALSE; // Aka no, I don’t want to block closing
    }

    dlg = gt_close_dialog_new (GT_CONTEXT_WINDOW, children);

    gtk_window_set_transient_for (GTK_WINDOW (dlg), GTK_WINDOW (self));

    g_signal_connect_swapped (dlg, "response::close", G_CALLBACK (close_response), self);

    gtk_widget_show (dlg);

    return TRUE; // Block the close
}


static void active_changed(GObject *object, GParamSpec *pspec, gpointer data)
{
    if (gtk_window_is_active (GTK_WINDOW (object))) {
        gt_watcher_push_active (gt_watcher_get_default ());
    } else {
        gt_watcher_pop_active (gt_watcher_get_default ());
    }
}


static void zoom(GtPages *pages, KgxZoom dir, GtWindow *self)
{
    GAction *action = NULL;
    GtkApplication *app = gtk_window_get_application (GTK_WINDOW (self));

    switch (dir) {
        case GT_ZOOM_IN:
            action = g_action_map_lookup_action (G_ACTION_MAP (app), "zoom-in");
            break;
        case GT_ZOOM_OUT:
            action = g_action_map_lookup_action (G_ACTION_MAP (app), "zoom-out");
            break;
        default:
            g_return_if_reached ();
    }
    g_action_activate (action, NULL);
}


static GtPages *create_tearoff_host(GtPages *pages, GtWindow *self)
{
    GtkApplication *application = gtk_window_get_application (GTK_WINDOW (self));
    GtWindow *new_window;
    int width, height;

    gtk_window_get_default_size (GTK_WINDOW (self), &width, &height);

    new_window = g_object_new (GT_TYPE_WINDOW, "application", application, "settings", self->settings, "default-width",
                               width, "default-height", height, NULL);
    gtk_window_present (GTK_WINDOW (new_window));

    return GT_PAGES (new_window->pages);
}


static void status_changed(GObject *object, GParamSpec *pspec, gpointer data)
{
    GtWindow *self = GT_WINDOW (object);
    GtStatus status;

    status = gt_pages_current_status (GT_PAGES (self->pages));

    if (status & GT_REMOTE) {
        gtk_widget_add_css_class (GTK_WIDGET (self), GT_WINDOW_STYLE_REMOTE);
    } else {
        gtk_widget_remove_css_class (GTK_WIDGET (self), GT_WINDOW_STYLE_REMOTE);
    }

    if (status & GT_PRIVILEGED) {
        gtk_widget_add_css_class (GTK_WIDGET (self), GT_WINDOW_STYLE_ROOT);
    } else {
        gtk_widget_remove_css_class (GTK_WIDGET (self), GT_WINDOW_STYLE_ROOT);
    }
}


static void extra_drag_drop(AdwTabBar *bar, AdwTabPage *page, GValue *value, GtWindow *self) 
{
    GtTab *tab = GT_TAB (adw_tab_page_get_child (page));

    gt_tab_accept_drop (tab, value);
}


static void new_tab_activated(GSimpleAction *action, GVariant *parameter, gpointer data);


static void new_tab_cb(GtTabSwitcher *switcher, GtWindow *self) 
{
    new_tab_activated (NULL, NULL, self);
}


static void gt_window_class_init(GtWindowClass *klass) 
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkWindowClass *window_class = GTK_WINDOW_CLASS (klass);

    object_class->dispose = gt_window_dispose;
    object_class->set_property = gt_window_set_property;
    object_class->get_property = gt_window_get_property;

    window_class->close_request = gt_window_close_request;

    pspecs[PROP_SETTINGS] = g_param_spec_object ("settings", NULL, NULL, GT_TYPE_SETTINGS,
                                                 G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, LAST_PROP, pspecs);

    gtk_widget_class_set_template_from_resource (widget_class, GT_APPLICATION_PATH "gt-window.ui");

    gtk_widget_class_bind_template_child (widget_class, GtWindow, window_title);
    gtk_widget_class_bind_template_child (widget_class, GtWindow, exit_info);
    gtk_widget_class_bind_template_child (widget_class, GtWindow, exit_message);
    gtk_widget_class_bind_template_child (widget_class, GtWindow, theme_switcher);
    gtk_widget_class_bind_template_child (widget_class, GtWindow, zoom_level);
    gtk_widget_class_bind_template_child (widget_class, GtWindow, tab_bar);
    gtk_widget_class_bind_template_child (widget_class, GtWindow, tab_button);
    gtk_widget_class_bind_template_child (widget_class, GtWindow, tab_switcher);
    gtk_widget_class_bind_template_child (widget_class, GtWindow, pages);
    gtk_widget_class_bind_template_child (widget_class, GtWindow, primary_menu);
    gtk_widget_class_bind_template_child (widget_class, GtWindow, settings_binds);

    gtk_widget_class_bind_template_callback (widget_class, active_changed);

    gtk_widget_class_bind_template_callback (widget_class, zoom);
    gtk_widget_class_bind_template_callback (widget_class, create_tearoff_host);
    gtk_widget_class_bind_template_callback (widget_class, status_changed);
    gtk_widget_class_bind_template_callback (widget_class, extra_drag_drop);
    gtk_widget_class_bind_template_callback (widget_class, new_tab_cb);
}


static gboolean scale_to_label(GBinding *binding, const GValue *from_value, GValue *to_value, gpointer user_data)
{
    int zoom = round (g_value_get_double (from_value) * 100);

    g_value_take_string (to_value, g_strdup_printf ("%i%%", zoom));

    return TRUE;
}


static void new_activated(GSimpleAction *action, GVariant *parameter, gpointer data) 
{
    GtWindow *self = data;
    guint32 timestamp = GDK_CURRENT_TIME;
    GtkApplication *application = NULL;
    g_autoptr (GFile) dir = NULL;

    application = gtk_window_get_application (GTK_WINDOW (self));
    dir = gt_window_get_working_dir (GT_WINDOW (data));

    gt_application_add_terminal (GT_APPLICATION (application), NULL, timestamp, dir, NULL, NULL);
}


static void new_tab_activated(GSimpleAction *action, GVariant *parameter, gpointer data)
{
    GtWindow *self = data;
    guint32 timestamp = GDK_CURRENT_TIME;
    GtkApplication *application = NULL;
    g_autoptr (GFile) dir = NULL;

    application = gtk_window_get_application (GTK_WINDOW (self));
    dir = gt_window_get_working_dir (GT_WINDOW (data));

    gt_application_add_terminal (GT_APPLICATION (application), self, timestamp, dir, NULL, NULL);
}


static void close_tab_activated(GSimpleAction *action, GVariant *parameter, gpointer data) 
{
    GtWindow *self = data;

    gt_pages_close_page (GT_PAGES (self->pages));
}


static void detach_tab_activated(GSimpleAction *action, GVariant *parameter, gpointer data) 
{
    GtWindow *self = data;

    gt_pages_detach_page (GT_PAGES (self->pages));
}


static void about_activated(GSimpleAction *action, GVariant *parameter, gpointer data) 
{
    const char *developers[] = {"Zander Brown <zbrown@gnome.org>", NULL};
    const char *designers[] = {"Tobias Bernard", NULL};
    g_autofree char *copyright = NULL;

    /* Translators: %s is the year range */
    copyright = g_strdup_printf (_("© %s Zander Brown"), "2019-2021");

    adw_show_about_window (GTK_WINDOW (data), "application-name", GT_DISPLAY_NAME, "application-icon",
                           GT_APPLICATION_ID, "developer-name", _("The GNOME Project"), "issue-url",
                           "https://gitlab.gnome.org/GNOME/console/-/issues/new", "version", PACKAGE_VERSION,
                           "developers", developers, "designers", designers,
        /* Translators: Credit yourself here */
                           "translator-credits", _("translator-credits"), "copyright", copyright, "license-type",
                           GTK_LICENSE_GPL_3_0, NULL);
}


static void tab_switcher_activated(GSimpleAction *action, GVariant *parameter, gpointer data)
{
    GtWindow *self = data;

    gt_tab_switcher_open (GT_TAB_SWITCHER (self->tab_switcher));
}


static GActionEntry win_entries[] = {{"new-window",   new_activated,          NULL, NULL, NULL},
                                     {"new-tab",      new_tab_activated,      NULL, NULL, NULL},
                                     {"close-tab",    close_tab_activated,    NULL, NULL, NULL},
                                     {"about",        about_activated,        NULL, NULL, NULL},
                                     {"tab-switcher", tab_switcher_activated, NULL, NULL, NULL},};


static GActionEntry tab_entries[] = {{"close",  close_tab_activated,  NULL, NULL, NULL},
                                     {"detach", detach_tab_activated, NULL, NULL, NULL},};


static gboolean update_title(GBinding *binding, const GValue *from_value, GValue *to_value, gpointer data)
{
    const char *title = g_value_get_string (from_value);

    if (!title) {
        title = GT_DISPLAY_NAME;
    }

    g_value_set_string (to_value, title);

    return TRUE;
}


static gboolean update_subtitle(GBinding *binding, const GValue *from_value, GValue *to_value, gpointer data) 
{
    g_autoptr (GFile) file = NULL;
    g_autofree char *path = NULL;
    const char *home = NULL;

    file = g_value_dup_object (from_value);
    if (file == NULL) {
        g_value_set_string (to_value, NULL);
        return TRUE;
    }

    path = g_file_get_path (file);
    if (path == NULL) {
        g_value_set_string (to_value, NULL);

        return TRUE;
    }

    home = g_get_home_dir ();
    if (g_str_has_prefix (path, home)) {
        g_autofree char *short_home = g_strdup_printf ("~%s", path + strlen (home));

        g_value_set_string (to_value, short_home);

        return TRUE;
    }

    g_value_set_string (to_value, path);

    return TRUE;
}


static void gt_window_init(GtWindow *self)
{
    g_autoptr (GtkWindowGroup) group = NULL;
    g_autoptr (GPropertyAction) pact = NULL;
    AdwStyleManager *style_manager;

    g_type_ensure (KGX_TYPE_TAB_BUTTON);
    g_type_ensure (GT_TYPE_TAB_SWITCHER);
    g_type_ensure (GT_TYPE_THEME_SWITCHER);

    gtk_widget_init_template (GTK_WIDGET (self));

    style_manager = adw_style_manager_get_default ();

    g_object_bind_property (style_manager, "system-supports-color-schemes", self->theme_switcher, "show-system",
                            G_BINDING_SYNC_CREATE);

    g_binding_group_bind (self->settings_binds, "theme", self->theme_switcher, "theme",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

    g_binding_group_bind_full (self->settings_binds, "font-scale", self->zoom_level, "label",
                               G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL, scale_to_label, NULL, NULL, NULL);

    g_action_map_add_action_entries (G_ACTION_MAP (self), win_entries, G_N_ELEMENTS (win_entries), self);

    pact = g_property_action_new ("find", G_OBJECT (self->pages), "search-mode-enabled");
    g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (pact));

#ifdef IS_DEVEL
    gtk_widget_add_css_class (GTK_WIDGET (self), "devel");
#endif

    g_object_bind_property_full (self->pages, "title", self, "title", G_BINDING_SYNC_CREATE, update_title, NULL, NULL,
                                 NULL);

    g_object_bind_property (self, "title", self->window_title, "title", G_BINDING_SYNC_CREATE);

    g_object_bind_property_full (self->pages, "path", self->window_title, "subtitle", G_BINDING_SYNC_CREATE,
                                 update_subtitle, NULL, NULL, NULL);

    g_object_bind_property (self->pages, "tab-view", self->tab_bar, "view", G_BINDING_SYNC_CREATE);
    g_object_bind_property (self->pages, "tab-view", self->tab_button, "view", G_BINDING_SYNC_CREATE);
    g_object_bind_property (self->pages, "tab-view", self->tab_switcher, "view", G_BINDING_SYNC_CREATE);

    adw_tab_bar_setup_extra_drop_target (ADW_TAB_BAR (self->tab_bar), GDK_ACTION_COPY, (GType[1]) {G_TYPE_STRING}, 1);

    group = gtk_window_group_new ();
    gtk_window_group_add_window (group, GTK_WINDOW (self));

    self->tab_actions = G_ACTION_MAP (g_simple_action_group_new ());
    g_action_map_add_action_entries (self->tab_actions, tab_entries, G_N_ELEMENTS (tab_entries), self);
    gtk_widget_insert_action_group (GTK_WIDGET (self), "tab", G_ACTION_GROUP (self->tab_actions));
}


GFile *gt_window_get_working_dir(GtWindow *self)
{
    GFile *file = NULL;

    g_return_val_if_fail (GT_IS_WINDOW (self), NULL);

    g_object_get (self->pages, "path", &file, NULL);

    return file;
}


GtPages *gt_window_get_pages(GtWindow *self)
{
    g_return_val_if_fail (GT_IS_WINDOW (self), NULL);

    return GT_PAGES (self->pages);
}
