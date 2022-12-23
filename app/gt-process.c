#include <gio/gio.h>
#include <glibtop/proclist.h>
#include <glibtop/procuid.h>
#include <glibtop/procargs.h>

#include "gt-process.h"

struct _GtProcess
{
    GPid    pid;
    GPid    parent;
    gint32  euid;
    GStrv   argv;
    char*   exec;
};

static void clear_process (GtProcess *self)
{
    g_clear_pointer (&self->argv, g_strfreev);
    g_clear_pointer (&self->exec, g_free);
}

void gt_process_unref (GtProcess *self)
{
    g_return_if_fail (self != NULL);

    g_rc_box_release_full (self, (GDestroyNotify) clear_process);
}

G_DEFINE_BOXED_TYPE (GtProcess, gt_process, g_rc_box_acquire, gt_process_unref)

inline GtProcess* gt_process_new (GPid pid)
{
  glibtop_proc_uid   info;
  GtProcess        *self = NULL;

  self = g_rc_box_new0 (GtProcess);

  self->pid = pid;

  glibtop_get_proc_uid (&info, pid);

  self->parent = info.ppid;
  self->euid = info.euid;

  return self;
}

inline GPid gt_process_get_pid (GtProcess *self)
{
  g_return_val_if_fail (self != NULL, 0);

  return self->pid;
}

inline gboolean gt_process_get_is_root (GtProcess *self)
{
  g_return_val_if_fail (self != NULL, FALSE);

  return self->euid == 0;
}

inline GPid gt_process_get_parent (GtProcess *self)
{
  g_return_val_if_fail (self != NULL, 0);

  return self->parent;
}

inline GStrv gt_process_get_argv (GtProcess *self)
{
  g_return_val_if_fail (self != NULL, NULL);

  if (G_LIKELY (self->argv == NULL)) {
    glibtop_proc_args args_size;

    self->argv = glibtop_get_proc_argv (&args_size, self->pid, 0);
  }

  return self->argv;
}

inline const char* gt_process_get_exec (GtProcess *self)
{
  g_return_val_if_fail (self != NULL, NULL);

  if (G_LIKELY (self->exec == NULL)) {
    self->exec = g_strjoinv (" ", gt_process_get_argv (self));
  }

  return self->exec;
}

int gt_pid_cmp (gconstpointer a, gconstpointer b, gpointer data)
{
  return a - b;
}

GTree* gt_process_get_list (void)
{
  glibtop_proclist pid_list;
  g_autofree GPid *pids = NULL;
  GTree *list = NULL;

  list = g_tree_new_full (gt_pid_cmp, NULL, NULL, (GDestroyNotify) gt_process_unref);

  pids = glibtop_get_proclist (&pid_list, GLIBTOP_KERN_PROC_ALL, 0);

  g_return_val_if_fail (pids != NULL, NULL);

  for (int i = 0; i < pid_list.number; i++) {
    g_tree_insert (list, GINT_TO_POINTER (pids[i]), gt_process_new (pids[i]));
  }

  return list;
}
