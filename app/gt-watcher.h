//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_WATCHER_H
#define GRACEFUL_TERMINATOR_GT_WATCHER_H

#include <glib-object.h>

#include "gt-tab.h"
#include "gt-process.h"

G_BEGIN_DECLS

#define GT_TYPE_WATCHER gt_watcher_get_type ()

G_DECLARE_FINAL_TYPE (GtWatcher, gt_watcher, GT, WATCHER, GObject)


GtWatcher *gt_watcher_get_default(void);

void gt_watcher_add(GtWatcher *self, GPid pid, GtTab *page);

void gt_watcher_remove(GtWatcher *self, GPid pid);

void gt_watcher_push_active(GtWatcher *self);

void gt_watcher_pop_active(GtWatcher *self);

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_WATCHER_H
