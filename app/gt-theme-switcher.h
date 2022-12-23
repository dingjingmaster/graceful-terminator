//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_THEME_SWITCHER_H
#define GRACEFUL_TERMINATOR_GT_THEME_SWITCHER_H
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GT_TYPE_THEME_SWITCHER (gt_theme_switcher_get_type())

G_DECLARE_FINAL_TYPE (GtThemeSwitcher, gt_theme_switcher, GT, THEME_SWITCHER, GtkWidget)

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_THEME_SWITCHER_H
