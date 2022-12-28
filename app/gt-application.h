//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_APPLICATION_H
#define GRACEFUL_TERMINATOR_GT_APPLICATION_H

#include <gtk/gtk.h>

#include "gt-tab.h"
#include "gt-window.h"
#include "gt-settings.h"

G_BEGIN_DECLS

#define GT_DISPLAY_NAME         _("Graceful Terminator")
#define GT_TYPE_APPLICATION     (gt_application_get_type())

struct _GtApplication
{
    /*< private >*/
    AdwApplication              parent_instance;

    /*< public >*/
    GTree*                      pages;
    GtSettings*                 settings;
};
G_DECLARE_FINAL_TYPE (GtApplication, gt_application, GT, APPLICATION, AdwApplication)


void    gt_application_add_page     (GtApplication* self, GtTab* page);
GtTab*  gt_application_lookup_page  (GtApplication* self, guint id);
GtTab*  gt_application_add_terminal (GtApplication* self, GtWindow* existingWindow, guint32 timestamp,
                                     GFile* workingDirectory, GStrv command, const char* title);

G_END_DECLS
#endif //GRACEFUL_TERMINATOR_GT_APPLICATION_H
