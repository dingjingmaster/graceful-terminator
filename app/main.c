#include "gt.h"

#include "gt-log.h"

#include <glib/gi18n.h>

static GLogWriterOutput log_handler (GLogLevelFlags level, const GLogField* fields, gsize nFields, gpointer udata);

int main(int argc, char *argv[])
{
    g_autoptr (GtkApplication) app = NULL;

    g_log_set_writer_func (log_handler, NULL, NULL);

    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

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

static GLogWriterOutput log_handler (GLogLevelFlags level, const GLogField* fields, gsize nFields, gpointer udata)
{
    if (NULL == g_getenv ("GRACEFUL_TERMINATOR_DEBUG")) {
        return G_LOG_WRITER_HANDLED;
    }

    const char* logLevel = NULL;

    g_autofree char* msg = NULL;
    g_autofree char* line = NULL;
    const char* file = NULL;
    const char* func = NULL;
    const char* domain = NULL;
    const char* loc = NULL;

    switch (level) {
        case G_LOG_LEVEL_DEBUG: {
            logLevel = "DEBUG";
            break;
        }
        case G_LOG_LEVEL_INFO: {
            logLevel = "INFO";
            break;
        }
        case G_LOG_LEVEL_WARNING: {
            logLevel = "WARNING";
            break;
        }
        case G_LOG_LEVEL_CRITICAL:
        case G_LOG_LEVEL_ERROR: {
            logLevel = "ERROR";
            break;
        }
        default: {
            logLevel = "UNKNOWN";
        }
    }

    for (int i = 0; i < nFields; ++i) {
        if (0 == g_ascii_strcasecmp ("file", fields[i].key)) {
            file = fields[i].value;
        }
        else if (0 == g_ascii_strcasecmp ("func", fields[i].key)) {
            func = fields[i].value;
        }
        else if (0 == g_ascii_strcasecmp ("line", fields[i].key)) {
            line = g_strdup_printf ("%d", fields[i].value);
        }
        else if (0 == g_ascii_strcasecmp ("message", fields[i].key)) {
            msg = g_strdup_printf (fields[i].value);
        }
        else if (0 == g_ascii_strcasecmp ("glib_domain", fields[i].key)) {
            domain = g_strdup_printf (fields[i].value);
        }
        else if (0 == g_ascii_strcasecmp ("loc", fields[i].key)) {
            loc = fields[i].value;
        }
#if 0
        else {
            write (2, fields[i].key, strlen (fields[i].key));
            write (2, "\n", 1);
        }
#endif
    }

    write (2, PACKAGE_NAME, strlen (PACKAGE_NAME));
    write (2, " ", 1);

    if (domain) {
        write (2, domain, strlen (domain));
        write (2, " ", 1);
    }

    {
        write (2, "[", 1);
        write (2, logLevel, strlen (logLevel));
        write (2, "] ", 2);
    }

    if (file) {
        write (2, file, strlen (file));
    }

    if (line) {
        write (2, ":", 1);
        write (2, line, strlen (line));
        write (2, " ", 1);
    }

    if (func) {
        write (2, func, strlen (func));
        write (2, " ", 1);
    }

    if (msg) {
        write (2, msg, strlen (msg));
    }

    write (2, "\n", 1);

    return G_LOG_WRITER_HANDLED;
}
