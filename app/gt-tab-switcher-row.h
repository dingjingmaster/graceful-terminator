//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_TAB_SWITCHER_ROW_H
#define GRACEFUL_TERMINATOR_GT_TAB_SWITCHER_ROW_H

#include <adwaita.h>

G_BEGIN_DECLS

#define GT_TYPE_TAB_SWITCHER_ROW (gt_tab_switcher_row_get_type())

G_DECLARE_FINAL_TYPE (GtTabSwitcherRow, gt_tab_switcher_row, GT, TAB_SWITCHER_ROW, GtkListBoxRow)

GtkWidget *gt_tab_switcher_row_new(AdwTabPage *page, AdwTabView *view);

AdwTabPage *gt_tab_switcher_row_get_page(GtTabSwitcherRow *self);

gboolean gt_tab_switcher_row_is_animating(GtTabSwitcherRow *self);

void gt_tab_switcher_row_animate_open(GtTabSwitcherRow *self);

void gt_tab_switcher_row_animate_close(GtTabSwitcherRow *self);

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_TAB_SWITCHER_ROW_H
