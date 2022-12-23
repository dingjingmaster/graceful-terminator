//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_TAB_SWITCHER_H
#define GRACEFUL_TERMINATOR_GT_TAB_SWITCHER_H

#include <adwaita.h>

G_BEGIN_DECLS

#define GT_TYPE_TAB_SWITCHER (gt_tab_switcher_get_type())

G_DECLARE_FINAL_TYPE (GtTabSwitcher, gt_tab_switcher, GT, TAB_SWITCHER, GtkWidget)

GtkWidget *gt_tab_switcher_new(void);

GtkWidget *gt_tab_switcher_get_child(GtTabSwitcher *self);

void gt_tab_switcher_set_child(GtTabSwitcher *self, GtkWidget *child);

AdwTabView *gt_tab_switcher_get_view(GtTabSwitcher *self);

void gt_tab_switcher_set_view(GtTabSwitcher *self, AdwTabView *view);

void gt_tab_switcher_open(GtTabSwitcher *self);

void gt_tab_switcher_close(GtTabSwitcher *self);

gboolean gt_tab_switcher_get_narrow(GtTabSwitcher *self);

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_TAB_SWITCHER_H
