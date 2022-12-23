//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_VTE_UTIL_H
#define GRACEFUL_TERMINATOR_GT_VTE_UTIL_H

#include <vte/vte.h>

G_BEGIN_DECLS

void gt_vte_pty_spawn_async (VtePty* pty, const char* working_directory, const char* const* argv, const char* const* env, int timeout, GCancellable* cancellable, GAsyncReadyCallback callback, gpointer udata);
gboolean gt_vte_pty_spawn_finish (VtePty* pty, GAsyncResult* result, GPid* child_pid, GError** error);

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_VTE_UTIL_H
