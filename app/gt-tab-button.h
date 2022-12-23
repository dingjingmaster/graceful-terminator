//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_TAB_BUTTON_H
#define GRACEFUL_TERMINATOR_GT_TAB_BUTTON_H

#include <adwaita.h>

G_BEGIN_DECLS

#define KGX_TYPE_TAB_BUTTON (gt_tab_button_get_type())

G_DECLARE_FINAL_TYPE (GtTabButton, gt_tab_button, GT, TAB_BUTTON, GtkButton)

GtkWidget *gt_tab_button_new(void);

AdwTabView *gt_tab_button_get_view(GtTabButton *self);

void gt_tab_button_set_view(GtTabButton *self, AdwTabView *view);

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_TAB_BUTTON_H
