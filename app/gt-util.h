//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_UTIL_H
#define GRACEFUL_TERMINATOR_GT_UTIL_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

void gt_util_transform_uris_to_quoted_fuse_paths(GStrv uris);

char* gt_util_concat_uris(GStrv uris, gsize *length);

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_UTIL_H
