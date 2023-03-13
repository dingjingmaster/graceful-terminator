#include "gt.h"

#include "gt-log.h"

#include <glib/gi18n.h>

const char* gLogPath = "/tmp/" PACKAGE_NAME ".log";

int main(int argc, char *argv[])
{
    g_autoptr (GtkApplication) app = NULL;

    textdomain (GETTEXT_PACKAGE);
    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    g_log_set_writer_func (log_handler, NULL, NULL);

    INFO(PACKAGE_NAME " is starting...");

    g_set_application_name (GT_DISPLAY_NAME);
    gtk_window_set_default_icon_name (GT_APPLICATION_ID);

    app = g_object_new (GT_TYPE_APPLICATION,
                        "application_id", GT_APPLICATION_ID,
                        "flags", G_APPLICATION_HANDLES_COMMAND_LINE
                                | G_APPLICATION_HANDLES_OPEN
                                | G_APPLICATION_CAN_OVERRIDE_APP_ID,
                        "register-session", TRUE,
                        NULL);

    int ret = g_application_run (G_APPLICATION (app), argc, argv);

    INFO(PACKAGE_NAME " is exited with %d!", ret);

    return ret;
}
