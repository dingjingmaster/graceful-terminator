//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_WINDOW_H
#define GRACEFUL_TERMINATOR_GT_WINDOW_H


#include <gtk/gtk.h>
#include <adwaita.h>

#include "gt-pages.h"
#include "gt-settings.h"

G_BEGIN_DECLS

#define GT_WINDOW_STYLE_ROOT "root"
#define GT_WINDOW_STYLE_REMOTE "remote"


#define GT_TYPE_WINDOW (gt_window_get_type())

struct _GtWindow
{
    /*< private >*/
    AdwApplicationWindow parent_instance;

    /*< public >*/
    GtSettings *settings;
    GBindingGroup *settings_binds;

    /* Size indicator */
    int last_cols;
    int last_rows;
    guint timeout;

    gboolean close_anyway;

    /* Template widgets */
    GtkWidget *window_title;
    GtkWidget *exit_info;
    GtkWidget *exit_message;
    GtkWidget *theme_switcher;
    GtkWidget *zoom_level;
    GtkWidget *tab_bar;
    GtkWidget *tab_button;
    GtkWidget *tab_switcher;
    GtkWidget *pages;
    GMenu *primary_menu;

    GActionMap *tab_actions;
};

G_DECLARE_FINAL_TYPE (GtWindow, gt_window, GT, WINDOW, AdwApplicationWindow)

GFile *gt_window_get_working_dir(GtWindow *self);

void gt_window_show_status(GtWindow *self, const char *status);

GtPages *gt_window_get_pages(GtWindow *self);

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_WINDOW_H
