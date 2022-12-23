#include "gt-config.h"

#include <glib/gi18n.h>

#include "gt-terminal.h"
#include "gt-vte-util.h"
#include "gt-proxy-info.h"
#include "gt-simple-tab.h"


G_DEFINE_TYPE (GtSimpleTab, gt_simple_tab, KGX_TYPE_TAB)

enum
{
    PROP_0, PROP_INITIAL_WORK_DIR, PROP_COMMAND, LAST_PROP
};
static GParamSpec *pspecs[LAST_PROP] = {NULL,};


static void gt_simple_tab_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    GtSimpleTab *self = GT_SIMPLE_TAB (object);

    switch (property_id) {
        case PROP_INITIAL_WORK_DIR:
            self->initial_work_dir = g_value_dup_string (value);
            break;
        case PROP_COMMAND:
            self->command = g_value_dup_boxed (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static void gt_simple_tab_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
    GtSimpleTab *self = GT_SIMPLE_TAB (object);

    switch (property_id) {
        case PROP_INITIAL_WORK_DIR:
            g_value_set_string (value, self->initial_work_dir);
            break;
        case PROP_COMMAND:
            g_value_set_boxed (value, self->command);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static void gt_simple_tab_finalize(GObject *object)
{
    GtSimpleTab *self = GT_SIMPLE_TAB (object);

    g_clear_pointer (&self->initial_work_dir, g_free);
    g_clear_pointer (&self->command, g_strfreev);

    if (self->spawn_cancellable) {
        g_cancellable_cancel (self->spawn_cancellable);
    }
    g_clear_object (&self->spawn_cancellable);

    G_OBJECT_CLASS (gt_simple_tab_parent_class)->finalize (object);
}


typedef struct
{
    GtSimpleTab *self;
    GTask *task;
} StartData;


static void clear_start_data(gpointer data)
{
    StartData *self = data;

    g_clear_weak_pointer (&self->self);
    g_clear_object (&self->task);

    g_free (self);
}


G_DEFINE_AUTOPTR_CLEANUP_FUNC (StartData, clear_start_data)


typedef struct
{
    GtSimpleTab *self;
} WaitData;


static void clear_wait_data(gpointer data)
{
    WaitData *self = data;

    g_clear_weak_pointer (&self->self);

    g_free (self);
}


G_DEFINE_AUTOPTR_CLEANUP_FUNC (WaitData, clear_wait_data)


static void wait_cb(GPid pid, int status, gpointer user_data)
{
    g_autoptr (WaitData) data = user_data;
    g_autoptr (GError) error = NULL;

    if (!data->self) {
        return; /* Tab destroyed before the process actually died */
    }

    g_return_if_fail (GT_SIMPLE_TAB (data->self));

    /* wait_check will set @error if it got a signal/non-zero exit */
    if (!g_spawn_check_wait_status (status, &error)) {
        g_autofree char *message = NULL;

        // translators: <b> </b> marks the text as bold, ensure they are
        // matched please!
        message = g_strdup_printf (_("<b>Read Only</b> — Command exited with code %i"), status);

        gt_tab_died (GT_TAB (data->self), GTK_MESSAGE_ERROR, message, TRUE);
    } else {
        gt_tab_died (GT_TAB (data->self), GTK_MESSAGE_INFO,
            // translators: <b> </b> marks the text as bold, ensure they are
            // matched please!
                     _("<b>Read Only</b> — Command exited"), TRUE);
    }
}


static void spawned(VtePty *pty, GAsyncResult *res, gpointer udata)
{
    g_autoptr (StartData) start_data = udata;
    g_autoptr (WaitData) wait_data = NULL;
    g_autoptr (GError) error = NULL;
    GPid pid;

    g_return_if_fail (VTE_IS_PTY (pty));
    g_return_if_fail (G_IS_ASYNC_RESULT (res));

    gt_vte_pty_spawn_finish (pty, res, &pid, &error);

    if (!start_data->self) {
        return; /* The tab went away whilst we were spawning */
    }

    g_return_if_fail (GT_SIMPLE_TAB (start_data->self));

    if (error) {
        g_autofree char *message = NULL;

        // translators: <b> </b> marks the text as bold, ensure they are
        // matched please!
        message = g_strdup_printf (_("<b>Failed to start</b> — %s"), error->message);

        gt_tab_died (GT_TAB (start_data->self), GTK_MESSAGE_ERROR, message, FALSE);

        g_task_return_error (start_data->task, g_steal_pointer (&error));

        return;
    }

    wait_data = g_new0 (WaitData, 1);
    g_set_weak_pointer (&wait_data->self, start_data->self);

    g_child_watch_add (pid, wait_cb, g_steal_pointer (&wait_data));

    g_task_return_int (G_TASK (start_data->task), pid);
}


static void gt_simple_tab_start(GtTab *page, GAsyncReadyCallback callback, gpointer callback_data)
{
    GtSimpleTab *self;
    g_autoptr (VtePty) pty = NULL;
    g_autoptr (GError) error = NULL;
    g_auto (GStrv) env = NULL;
    g_autoptr (StartData) data = NULL;
    g_autoptr (GTask) task = NULL;

    g_return_if_fail (GT_IS_SIMPLE_TAB (page));

    self = GT_SIMPLE_TAB (page);

    if (self->spawn_cancellable) {
        g_cancellable_reset (self->spawn_cancellable);
    } else {
        self->spawn_cancellable = g_cancellable_new ();
    }

    task = g_task_new (self, self->spawn_cancellable, callback, callback_data);
    g_task_set_source_tag (task, gt_simple_tab_start);

    pty = vte_pty_new_sync (VTE_PTY_DEFAULT, self->spawn_cancellable, &error);
    if (error) {
        g_task_return_error (task, error);
        g_clear_object (&self->spawn_cancellable);

        return;
    }

    env = g_environ_setenv (env, "TERM", "xterm-256color", TRUE);

    vte_terminal_set_pty (VTE_TERMINAL (self->terminal), pty);

    data = g_new0 (StartData, 1);
    g_set_weak_pointer (&data->self, self);
    g_set_object (&data->task, task);

    gt_proxy_info_apply_to_environ (gt_proxy_info_get_default (), &env);

    gt_vte_pty_spawn_async (pty, self->initial_work_dir, (const char *const *) self->command, (const char *const *) env,
                            -1, self->spawn_cancellable, (GAsyncReadyCallback) spawned, g_steal_pointer (&data));
}


static GPid gt_simple_tab_start_finish(GtTab *page, GAsyncResult *res, GError **error)
{
    g_return_val_if_fail (g_task_is_valid (res, page), 0);

    return g_task_propagate_int (G_TASK (res), error);
}


static void path_changed(GObject *object, GParamSpec *pspec, GtSimpleTab *self)
{
    g_autofree char *desc = NULL;
    g_autoptr (GFile) path = NULL;

    g_object_get (self->terminal, "path", &path, NULL);

    if (path) {
        desc = g_strdup_printf ("%s", g_file_get_path (path));
    }

    g_object_set (self, "tab-tooltip", desc, NULL);
}


static void gt_simple_tab_class_init(GtSimpleTabClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS   (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtTabClass *page_class = GT_TAB_CLASS (klass);

    object_class->set_property = gt_simple_tab_set_property;
    object_class->get_property = gt_simple_tab_get_property;
    object_class->finalize = gt_simple_tab_finalize;

    page_class->start = gt_simple_tab_start;
    page_class->start_finish = gt_simple_tab_start_finish;

    /**
     * GtSimpleTab:initial-work-dir:
     *
     * Used to handle --working-dir
     */
    pspecs[PROP_INITIAL_WORK_DIR] = g_param_spec_string ("initial-work-dir", "Initial directory",
                                                         "Initial working directory", NULL,
                                                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    /**
     * GtSimpleTab:command:
     *
     * Used to handle -e
     */
    pspecs[PROP_COMMAND] = g_param_spec_boxed ("command", "Command", "Command to run", G_TYPE_STRV,
                                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    g_object_class_install_properties (object_class, LAST_PROP, pspecs);

    gtk_widget_class_set_template_from_resource (widget_class, GT_APPLICATION_PATH "kgx-simple-tab.ui");

    gtk_widget_class_bind_template_child (widget_class, GtSimpleTab, terminal);

    gtk_widget_class_bind_template_callback (widget_class, path_changed);
}


static void gt_simple_tab_init(GtSimpleTab *self)
{
    gtk_widget_init_template (GTK_WIDGET (self));
}
