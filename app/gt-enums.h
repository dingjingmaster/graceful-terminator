//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_ENUMS_H
#define GRACEFUL_TERMINATOR_GT_ENUMS_H
#include <glib-object.h>

G_BEGIN_DECLS

GType gt_close_dialog_context_get_type (void);
#define GT_TYPE_CLOSE_DIALOG_CONTEXT (gt_close_dialog_context_get_type())

GType gt_theme_get_type (void);
#define GT_TYPE_THEME (gt_theme_get_type())

GType gt_status_get_type (void);
#define GT_TYPE_STATUS (gt_status_get_type())


GType gt_zoom_get_type (void);
#define GT_TYPE_ZOOM (gt_zoom_get_type())

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_ENUMS_H
