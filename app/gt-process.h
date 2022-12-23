//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_PROCESS_H
#define GRACEFUL_TERMINATOR_GT_PROCESS_H

#include <glib.h>
#include <glib-object.h>

#include "gt-config.h"

G_BEGIN_DECLS

typedef struct _GtProcess GtProcess;

#define GT_TYPE_PROCESS (gt_process_get_type ())

GTree*      gt_process_get_list    (void);
GtProcess*  gt_process_new         (GPid        pid);
GPid        gt_process_get_pid     (GtProcess *self);
gboolean    gt_process_get_is_root (GtProcess *self);
GPid        gt_process_get_parent  (GtProcess *self);
GStrv       gt_process_get_argv    (GtProcess *self);
const char* gt_process_get_exec    (GtProcess *self);
GType       gt_process_get_type    (void);
void        gt_process_unref       (GtProcess *self);
int         gt_pid_cmp             (gconstpointer a, gconstpointer b, gpointer data);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GtProcess, gt_process_unref)

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_PROCESS_H
