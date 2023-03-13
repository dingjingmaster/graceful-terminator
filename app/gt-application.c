//
// Created by dingjing on 12/22/22.
//
#include "gt-application.h"

#include <unistd.h>
#include <vte/vte.h>
#include <sys/ioctl.h>
#include <glib/gi18n.h>

#include "rgba.h"
#include "gt-log.h"
#include "gt-pages.h"
#include "gt-config.h"
#include "gt-window.h"
#include "gt-watcher.h"
#include "gt-resources.h"
#include "gt-simple-tab.h"

#define LOGO_COL_SIZE 28
#define LOGO_ROW_SIZE 14

G_DEFINE_TYPE (GtApplication, gt_application, ADW_TYPE_APPLICATION)


static void gt_application_finalize (GObject *object)
{
    GtApplication *self = GT_APPLICATION (object);

    g_clear_object (&self->settings);
    g_clear_pointer (&self->pages, g_tree_unref);

    G_OBJECT_CLASS (gt_application_parent_class)->finalize (object);
}


static void gt_application_activate (GApplication *app)
{
    guint32 timestamp = GDK_CURRENT_TIME;

    GtkWindow* window = gtk_application_get_active_window (GTK_APPLICATION (app));
    if (window == NULL) {
        DEBUG("There is no active window");
        gt_application_add_terminal (GT_APPLICATION (app), NULL, timestamp, NULL, NULL, NULL);
        return;
    }

    DEBUG("opened!");

    // 将焦点给到打开的窗口
    gtk_window_present_with_time (window, timestamp);
}


static void gt_application_startup (GApplication *app)
{
    const char *const newWindowAccel[] =    {"<shift><primary>n",  NULL};
    const char *const newTabAccel[] =       {"<shift><primary>t",  NULL};
    const char *const closeTabAccel[] =     {"<shift><primary>w",  NULL};
    const char *const copyAccel[] =         {"<shift><primary>c",  NULL};
    const char *const pasteAccel[] =        {"<shift><primary>v",  NULL};
    const char *const findAccel[] =         {"<shift><primary>f",  NULL};
    const char *const zoomInAccel[] =       {"<primary>plus",      NULL};
    const char *const zoomOutAccel[] =      {"<primary>minus",     NULL};
    const char *const zoomNormalAccel[] =   {"<primary>0",         NULL};

    g_resources_register (gt_get_resource ());

    g_type_ensure (GT_TYPE_TERMINAL);
    g_type_ensure (GT_TYPE_PAGES);

    G_APPLICATION_CLASS (gt_application_parent_class)->startup (app);

    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.new-window", newWindowAccel);
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.new-tab", newTabAccel);
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.close-tab", closeTabAccel);
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "term.copy", copyAccel);
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "term.paste", pasteAccel);
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.find", findAccel);
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.zoom-in", zoomInAccel);
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.zoom-out", zoomOutAccel);
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.zoom-normal", zoomNormalAccel);

    DEBUG("Set accel for action OK!");
}


static void gt_application_open (GApplication* app, GFile** files, int nFiles, const char* hint)
{
    DEBUG("")
    guint32 timestamp = GDK_CURRENT_TIME;

    for (int i = 0; i < nFiles; ++i) {
        gt_application_add_terminal (GT_APPLICATION (app), NULL, timestamp, files[i], NULL, NULL);
    }
}


static int gt_application_local_command_line (GApplication* app, char*** arguments, int* exitStatus)
{
    for (size_t i = 0; NULL != (*arguments)[i]; ++i) {
        if (i == 0) {
            continue;
        }

        if (strcmp ((*arguments)[i], "-e") == 0) {
            if (!((*arguments)[i + 1] != NULL && (*arguments)[i + 2] == NULL)) {
                (*arguments)[i][1] = '-';
            }

            break;
        } else if (strcmp ((*arguments)[i], "--") == 0) {
            break;
        }
    }

    return G_APPLICATION_CLASS (gt_application_parent_class)->local_command_line (app, arguments, exitStatus);
}

static int gt_application_command_line (GApplication* app, GApplicationCommandLine *cli)
{
    GtApplication *self = GT_APPLICATION (app);
    guint32 timestamp = GDK_CURRENT_TIME;
    GVariantDict *options = NULL;
    g_auto (GStrv) argv = NULL;
    const char *command = NULL;
    const char *workingDir = NULL;
    const char *title = NULL;
    const char *const *shell = NULL;
    const char *cwd = NULL;
    gint64 scrollBack;
    gboolean tab;
    g_autoptr (GFile) path = NULL;

    options = g_application_command_line_get_options_dict (cli);
    cwd = g_application_command_line_get_cwd (cli);

    g_variant_dict_lookup (options, "working-directory", "^&ay", &workingDir);
    g_variant_dict_lookup (options, "title", "&s", &title);
    g_variant_dict_lookup (options, "command", "^&ay", &command);
    g_variant_dict_lookup (options, G_OPTION_REMAINING, "^aay", &argv);

    if (g_variant_dict_lookup (options, "set-shell", "^as", &shell) && shell) {
        gt_settings_set_custom_shell (self->settings, shell);

        return EXIT_SUCCESS;
    }

    if (g_variant_dict_lookup (options, "set-scrollback", "x", &scrollBack)) {
        gt_settings_set_scrollback (self->settings, scrollBack);

        return EXIT_SUCCESS;
    }

    if (workingDir != NULL) {
        path = g_file_new_for_commandline_arg_and_cwd (workingDir, cwd);
    }

    if (path == NULL) {
        path = g_file_new_for_path (cwd);
    }

    if (command != NULL) {
        gboolean can_exec_directly;

        if (argv != NULL && argv[0] != NULL) {
            g_warning (_("Cannot use both --command and positional parameters"));
            return EXIT_FAILURE;
        }

        g_clear_pointer (&argv, g_strfreev);

        if (strchr (command, '/') != NULL) {
            can_exec_directly = g_file_test (command, G_FILE_TEST_IS_EXECUTABLE);
        } else {
            g_autofree char *program = g_find_program_in_path (command);

            can_exec_directly = (program != NULL);
        }


        if (can_exec_directly) {
            argv = g_new0 (char *, 2);
            argv[0] = g_strdup (command);
            argv[1] = NULL;
        } else {
            argv = g_new0 (char *, 4);
            argv[0] = g_strdup ("/bin/sh");
            argv[1] = g_strdup ("-c");
            argv[2] = g_strdup (command);
            argv[3] = NULL;
        }
    }

    if (g_variant_dict_lookup (options, "tab", "b", &tab) && tab) {
        gt_application_add_terminal (self, GT_WINDOW (gtk_application_get_active_window (GTK_APPLICATION (self))), timestamp, path, argv, title);
    } else {
        gt_application_add_terminal (self, NULL, timestamp, path, argv, title);
    }

    return EXIT_SUCCESS;
}


static void print_center (char *msg, int ign, short width)
{
    int halfMsg = (int) strlen (msg) / 2;
    int halfScreen = width / 2;

    g_print ("%*s\n", halfScreen + halfMsg, msg);
}

static void print_logo (short width)
{
    g_autoptr (GFile) logo = NULL;
    g_autoptr (GError) error = NULL;
    g_auto (GStrv) logoLines = NULL;
    g_autofree char *logoText = NULL;
    int i = 0;
    int halfScreen = width / 2;

    logo = g_file_new_for_uri ("resource:/" GT_APPLICATION_PATH "logo.txt");

    g_file_load_contents (logo, NULL, &logoText, NULL, NULL, &error);

    if (error) {
        g_error ("Wat? %s", error->message);
    }

    logoLines = g_strsplit (logoText, "\n", -1);

    while (logoLines[i]) {
        g_print ("%*s%s\n", halfScreen - (LOGO_COL_SIZE / 2), "", logoLines[i]);
        i++;
    }
}

static int gt_application_handle_local_options (GApplication *app, GVariantDict *options)
{
    gboolean version = FALSE;
    gboolean about = FALSE;
    g_autoptr(GDate) date = g_date_new();

    if (g_variant_dict_lookup (options, "version", "b", &version)) {
        if (version) {
            g_print (_("# graceful-terminator %s using VTE %u.%u.%u %s\n"),
                     PACKAGE_VERSION,
                     vte_get_major_version (),
                     vte_get_minor_version (),
                     vte_get_micro_version (),
                     vte_get_features ());
            return EXIT_SUCCESS;
        }
    }

    if (g_variant_dict_lookup (options, "about", "b", &about)) {
        if (about) {
            int year = g_date_get_year (date) > 2022 ? g_date_get_year (date) : 2022;
            g_autofree char *copyright = g_strdup_printf (_("© 2022-%d Ding Jing"), year);
            struct winsize w;

            ioctl (STDOUT_FILENO, TIOCGWINSZ, &w);
            int padding = ((w.ws_row -1) - (LOGO_ROW_SIZE + 5)) / 2;
            for (int i = 0; i < padding; i++) {
                g_print ("\n");
            }

            print_logo (w.ws_col);
            print_center (_("King’s Cross"), -1, w.ws_col);
            print_center (PACKAGE_VERSION, -1, w.ws_col);
            print_center (_("Graceful Terminal"), -1, w.ws_col);
            print_center (copyright, -1, w.ws_col);
            print_center (_("MIT"), -1, w.ws_col);

            for (int i = 0; i < padding; i++) {
                g_print ("\n");
            }

            return EXIT_SUCCESS;
        }
    }

    return G_APPLICATION_CLASS (gt_application_parent_class)->handle_local_options (app, options);
}


static void gt_application_class_init (GtApplicationClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

    object_class->finalize = gt_application_finalize;
    app_class->activate = gt_application_activate;
    app_class->startup = gt_application_startup;
    app_class->open = gt_application_open;
    app_class->local_command_line = gt_application_local_command_line;
    app_class->command_line = gt_application_command_line;
    app_class->handle_local_options = gt_application_handle_local_options;
}


static GOptionEntry entries[] =
{
    {"version", 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL},
    {"about", 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL},
    {"tab", 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL},
    {"command", 'e', 0, G_OPTION_ARG_FILENAME, NULL, N_("Execute the argument to this option inside the terminal"), N_("COMMAND")},
    {"working-directory", 0, 0, G_OPTION_ARG_FILENAME, NULL, N_("Set the working directory"), N_("DIRNAME")},
    {"wait", 0, 0, G_OPTION_ARG_NONE, NULL, N_("Wait until the child exits (TODO)"), NULL},
    {"title", 'T', 0, G_OPTION_ARG_STRING, NULL, N_("Set the initial window title"), N_("TITLE")},
    {"set-shell", 0, 0, G_OPTION_ARG_STRING_ARRAY, NULL, N_("ADVANCED: Set the shell to launch"), N_("SHELL")},
    {"set-scrollback", 0, 0, G_OPTION_ARG_INT64, NULL, N_("ADVANCED: Set the scrollback length"), N_("LINES")},
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, NULL, NULL, N_("[-e|-- COMMAND [ARGUMENT...]]")},
    { NULL }
};


static void new_window_activated (GSimpleAction* action, GVariant* parameter, gpointer data)
{
    GtApplication *self = GT_APPLICATION (data);
    guint32 timestamp = GDK_CURRENT_TIME;

    gt_application_add_terminal (self, NULL, timestamp, NULL, NULL, NULL);
}


static void new_tab_activated (GSimpleAction* action, GVariant* parameter, gpointer data)
{
    GtApplication *self = GT_APPLICATION (data);
    guint32 timestamp = GDK_CURRENT_TIME;
    g_autoptr (GFile) dir = NULL;
    GtkWindow* window = gtk_application_get_active_window (GTK_APPLICATION (self));
    if (window) {
        dir = gt_window_get_working_dir (GT_WINDOW (window));
        gt_application_add_terminal (self, GT_WINDOW (window), timestamp, dir, NULL, NULL);
    } else {
        gt_application_add_terminal (self, NULL, timestamp, NULL, NULL, NULL);
    }
}


static void focus_activated (GSimpleAction* action, GVariant* parameter, gpointer data)
{
    GtApplication *self = GT_APPLICATION (data);
    GtTab *page = gt_application_lookup_page (self, g_variant_get_uint32 (parameter));
    GtPages *pages = gt_tab_get_pages (page);
    gt_pages_focus_page (pages, page);
    GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (pages));

    gtk_window_present_with_time (GTK_WINDOW (root), GDK_CURRENT_TIME);
}


static void zoom_out_activated (GSimpleAction* action, GVariant* parameter, gpointer data)
{
    GtApplication *self = GT_APPLICATION (data);

    gt_settings_decrease_scale (self->settings);
}


static void zoom_normal_activated (GSimpleAction* action, GVariant* parameter, gpointer data)
{
    GtApplication *self = GT_APPLICATION (data);

    gt_settings_reset_scale (self->settings);
}



static void zoom_in_activated (GSimpleAction* action, GVariant* parameter, gpointer data)
{
    GtApplication *self = GT_APPLICATION (data);

    gt_settings_increase_scale (self->settings);
}


static GActionEntry app_entries[] =
{
    { "new-window", new_window_activated, NULL, NULL, NULL },
    { "new-tab", new_tab_activated, NULL, NULL, NULL },
    { "focus-page", focus_activated, "u", NULL, NULL },
    { "zoom-out", zoom_out_activated, NULL, NULL, NULL },
    { "zoom-normal", zoom_normal_activated, NULL, NULL, NULL },
    { "zoom-in", zoom_in_activated, NULL, NULL, NULL },
};


static gboolean theme_to_colour_scheme (GBinding* binding, const GValue* fromValue, GValue* toValue, gpointer udata)
{
    switch (g_value_get_enum (fromValue)) {
        case GT_THEME_AUTO:
            g_value_set_enum (toValue, ADW_COLOR_SCHEME_PREFER_LIGHT);
            break;
        case GT_THEME_DAY:
            g_value_set_enum (toValue, ADW_COLOR_SCHEME_FORCE_LIGHT);
            break;
        case GT_THEME_NIGHT:
        case GT_THEME_HACKER:
        default:
            g_value_set_enum (toValue, ADW_COLOR_SCHEME_FORCE_DARK);
            break;
    }

    return TRUE;
}


static gboolean scale_to_can_reset (GBinding* binding, const GValue* fromValue, GValue* toValue, gpointer udata)
{
    double scale = g_value_get_double (fromValue);

    g_value_set_boolean (toValue, fabs (scale - GT_FONT_SCALE_DEFAULT) > 0.05);

    return TRUE;
}

static void gt_application_init (GtApplication *self)
{
    g_autoptr (GPropertyAction) themeAction = NULL;
    AdwStyleManager *styleManager = adw_style_manager_get_default ();
    GAction *action;

    g_application_add_main_option_entries (G_APPLICATION (self), entries);

    g_action_map_add_action_entries (G_ACTION_MAP (self), app_entries, G_N_ELEMENTS (app_entries), self);

    self->settings = g_object_new (GT_TYPE_SETTINGS, NULL);
    g_object_bind_property_full (self->settings, "theme", styleManager, "color-scheme", G_BINDING_SYNC_CREATE, theme_to_colour_scheme, NULL, NULL, NULL);

    action = g_action_map_lookup_action (G_ACTION_MAP (self), "zoom-out");
    g_object_bind_property (self->settings, "scale-can-decrease", action, "enabled", G_BINDING_SYNC_CREATE);

    action = g_action_map_lookup_action (G_ACTION_MAP (self), "zoom-normal");
    g_object_bind_property_full (self->settings, "font-scale", action, "enabled", G_BINDING_SYNC_CREATE, scale_to_can_reset, NULL, NULL, NULL);

    action = g_action_map_lookup_action (G_ACTION_MAP (self), "zoom-in");
    g_object_bind_property (self->settings, "scale-can-increase", action, "enabled", G_BINDING_SYNC_CREATE);

    themeAction = g_property_action_new ("theme", self->settings, "theme");
    g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (themeAction));

    self->pages = g_tree_new_full (gt_pid_cmp, NULL, NULL, NULL);
}


static void page_died (gpointer data, GObject *deadObject)
{
    GtApplication *self = GT_APPLICATION (g_application_get_default ());

    g_tree_remove (self->pages, data);
}

void gt_application_add_page (GtApplication* self, GtTab* page)
{
    g_return_if_fail (GT_IS_APPLICATION (self));
    g_return_if_fail (GT_IS_TAB (page));

    guint id = gt_tab_get_id (page);

    g_tree_insert (self->pages, GINT_TO_POINTER (id), page);
    g_object_weak_ref (G_OBJECT (page), page_died, GINT_TO_POINTER (id));
}


GtTab* gt_application_lookup_page (GtApplication* self, guint id)
{
    g_return_val_if_fail (GT_IS_APPLICATION (self), NULL);

    return g_tree_lookup (self->pages, GUINT_TO_POINTER (id));
}


static void started (GObject* src, GAsyncResult* res, gpointer app)
{
    g_autoptr (GError) error = NULL;
    GtTab *page = GT_TAB (src);

    GPid pid = gt_tab_start_finish (page, res, &error);
    if (error) {
        g_warning ("Failed to start %s: %s", G_OBJECT_TYPE_NAME (src), error->message);
        return;
    }

    gt_watcher_add (gt_watcher_get_default (), pid, page);
}

GtTab* gt_application_add_terminal (GtApplication* self, GtWindow* existingWindow, guint32 timestamp, GFile* workingDirectory, GStrv argv, const char* title)
{
    g_autofree char *directory = NULL;
    g_auto (GStrv) shell = NULL;
    GtkWindow *window;
    GtkWidget *tab;
    GtPages *pages;

    if (G_LIKELY (argv == NULL)) {
        shell = gt_settings_get_shell (self->settings);
    }

    if (workingDirectory) {
        directory = g_file_get_path (workingDirectory);
    } else {
        directory = g_strdup (g_get_home_dir ());
    }

    DEBUG("Home dir: %s", directory);

    tab = g_object_new (GT_TYPE_SIMPLE_TAB,
                        "application", self,
                        "settings", self->settings,
                        "initial-work-dir", directory,
                        "command", shell != NULL ? shell : argv,
                        "tab-title", title,
                        "close-on-quit", argv == NULL, NULL);
    gt_tab_start (GT_TAB (tab), started, self);

    if (existingWindow) {
        window = GTK_WINDOW (existingWindow);
    } else {
        GtkWindow *active_window;
        int width = -1, height = -1;

        gt_settings_get_size (self->settings, &width, &height);

        active_window = gtk_application_get_active_window (GTK_APPLICATION (self));
        if (active_window) {
            gtk_window_get_default_size (active_window, &width, &height);
        }

        DEBUG("new window (%i×%i)", width, height);

        window = g_object_new (GT_TYPE_WINDOW,
                               "application", self,
                               "settings", self->settings,
                               "default-width", width,
                               "default-height", height,
                               NULL);
    }

    pages = gt_window_get_pages (GT_WINDOW (window));

    gt_pages_add_page (pages, GT_TAB (tab));
    gt_pages_focus_page (pages, GT_TAB (tab));

    gtk_window_present_with_time (window, timestamp);

    return GT_TAB (tab);
}


