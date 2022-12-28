//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_TAB_H
#define GRACEFUL_TERMINATOR_GT_TAB_H

#include <gtk/gtk.h>

#include "gt-terminal.h"
#include "gt-process.h"
#include "gt-enums.h"

G_BEGIN_DECLS

typedef enum
{
    GT_NONE = 0,              /*< nick=none >*/
    GT_REMOTE = (1 << 0),     /*< nick=remote >*/
    GT_PRIVILEGED = (1 << 1), /*< nick=privileged >*/
} GtStatus;

typedef enum
{
    GT_ZOOM_IN = 0,  /*< nick=in >*/
    GT_ZOOM_OUT = 1, /*< nick=out >*/
} GtZoom;


#ifndef __GTK_DOC_IGNORE__
typedef struct _GtPages GtPages;
#endif

#define GT_TYPE_TAB gt_tab_get_type ()

G_DECLARE_DERIVABLE_TYPE (GtTab, gt_tab, GT, TAB, GtkBox)


struct _GtTabClass
{
    /*< private >*/
    GtkBoxClass parent;

    /*< public >*/
    void (*start)(GtTab *tab, GAsyncReadyCallback callback, gpointer callback_data);

    GPid (*start_finish)(GtTab *tab, GAsyncResult *res, GError **error);

    void (*died)(GtTab *self, GtkMessageType type, const char *message, gboolean success);
};


guint gt_tab_get_id(GtTab *self);

void gt_tab_start(GtTab *self, GAsyncReadyCallback callback, gpointer callback_data);

GPid gt_tab_start_finish(GtTab *self, GAsyncResult *res, GError **error);

void gt_tab_died(GtTab *self, GtkMessageType type, const char *message, gboolean success);

GtPages *gt_tab_get_pages(GtTab *self);

void gt_tab_push_child(GtTab *self, GtProcess *process);

void gt_tab_pop_child(GtTab *self, GtProcess *process);

gboolean gt_tab_is_active(GtTab *self);

GPtrArray *gt_tab_get_children(GtTab *self);

void gt_tab_accept_drop(GtTab *self, const GValue *value);

void gt_tab_set_initial_title(GtTab *self, const char *title, GFile *path);

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_TAB_H
