//
// Created by dingjing on 23-4-18.
//

#ifndef GRACEFUL_TERMINATOR_GT_CONTEXT_MENU_H
#define GRACEFUL_TERMINATOR_GT_CONTEXT_MENU_H
#include <gtk/gtk.h>

#define GT_TYPE_CONTEXT_MENU        (gt_context_menu_get_type())

G_DECLARE_FINAL_TYPE(GtContextMenu, gt_context_menu, GT, CONTEXT_MENU, GtkPopover)

GtkWidget* gt_context_menu_new (void);

#endif //GRACEFUL_TERMINATOR_GT_CONTEXT_MENU_H
