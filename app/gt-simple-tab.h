//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_SIMPLE_TAB_H
#define GRACEFUL_TERMINATOR_GT_SIMPLE_TAB_H

#include <gtk/gtk.h>

#include "gt-tab.h"

G_BEGIN_DECLS

#define GT_TYPE_SIMPLE_TAB gt_simple_tab_get_type ()

struct _GtSimpleTab
{
    /*< private >*/
    GtTab     parent_instance;

    /*< public >*/
    char      *title;
    GFile     *path;

    char      *initial_work_dir;
    GStrv      command;

    GtkWidget *terminal;
    GCancellable *spawn_cancellable;
};

G_DECLARE_FINAL_TYPE (GtSimpleTab, gt_simple_tab, GT, SIMPLE_TAB, GtTab)


G_END_DECLS


#endif //GRACEFUL_TERMINATOR_GT_SIMPLE_TAB_H
