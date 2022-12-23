//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_PAGES_H
#define GRACEFUL_TERMINATOR_GT_PAGES_H

#include <gtk/gtk.h>
#include <adwaita.h>

#include "gt-tab.h"

G_BEGIN_DECLS

#define GT_TYPE_PAGES (gt_pages_get_type())

struct _GtPagesClass
{
    AdwBinClass parent;
};

G_DECLARE_DERIVABLE_TYPE (GtPages, gt_pages, GT, PAGES, AdwBin)


void        gt_pages_add_page       (GtPages* self, GtTab* page);
void        gt_pages_remove_page    (GtPages* self, GtTab* page);
int         gt_pages_count          (GtPages* self);
GPtrArray*  gt_pages_get_children   (GtPages* self);
void        gt_pages_focus_page     (GtPages* self, GtTab* page);
GtStatus    gt_pages_current_status (GtPages* self);
void        gt_pages_close_page     (GtPages* self);
void        gt_pages_detach_page    (GtPages* self);

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_PAGES_H
