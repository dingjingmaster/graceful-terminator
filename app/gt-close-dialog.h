//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_CLOSE_DIALOG_H
#define GRACEFUL_TERMINATOR_GT_CLOSE_DIALOG_H

#include <gtk/gtk.h>
#include "gt-enums.h"

G_BEGIN_DECLS

typedef enum
{
    GT_CONTEXT_WINDOW,
    GT_CONTEXT_TAB,
} GtCloseDialogContext;

GtkWidget* gt_close_dialog_new (GtCloseDialogContext context, GPtrArray* commands);

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_CLOSE_DIALOG_H
