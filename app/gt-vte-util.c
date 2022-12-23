//
// Created by dingjing on 12/22/22.
//
#include "gt-vte-util.h"


static void fp_vte_pty_spawn_cb(VtePty *pty, GAsyncResult *result, gpointer user_data)
{
    g_autoptr (GTask) task = user_data;
    g_autoptr (GError) error = NULL;
    GPid child_pid;

    g_assert (VTE_IS_PTY (pty));
    g_assert (G_IS_ASYNC_RESULT (result));
    g_assert (G_IS_TASK (task));

    if (!vte_pty_spawn_finish (pty, result, &child_pid, &error)) {
        g_task_return_error (task, g_steal_pointer (&error));
    } else {
        g_task_return_int (task, child_pid);
    }
}

void gt_vte_pty_spawn_async(VtePty *pty, const char *working_directory, const char *const *argv, const char *const *env,
                            int timeout, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data)
{
    g_autoptr (GTask) task = NULL;
    g_auto (GStrv) copy_env = NULL;

    g_return_if_fail (VTE_IS_PTY (pty));
    g_return_if_fail (argv != NULL);
    g_return_if_fail (argv[0] != NULL);

    if (timeout < 0) {
        timeout = -1;
    }

    if (working_directory == NULL) {
        working_directory = g_get_home_dir ();
    }

    if (env == NULL) {
        copy_env = g_get_environ ();
        env = (const char *const *) copy_env;
    }

    task = g_task_new (pty, cancellable, callback, user_data);
    g_task_set_source_tag (task, gt_vte_pty_spawn_async);

    vte_pty_spawn_async (pty, working_directory, (char **) argv, (char **) env,
                         G_SPAWN_SEARCH_PATH | G_SPAWN_SEARCH_PATH_FROM_ENVP, NULL, NULL, NULL, -1, cancellable,
                         (GAsyncReadyCallback) fp_vte_pty_spawn_cb, g_steal_pointer (&task));
}

gboolean gt_vte_pty_spawn_finish(VtePty *pty, GAsyncResult *result, GPid *child_pid, GError **error)
{
    GPid pid;

    g_return_val_if_fail (VTE_IS_PTY (pty), FALSE);
    g_return_val_if_fail (G_IS_TASK (result), FALSE);

    pid = g_task_propagate_int (G_TASK (result), error);

    if (pid > 0) {
        if (child_pid != NULL) {
            *child_pid = pid;
        }

        return TRUE;
    }

    return FALSE;
}
