//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_PROXY_INFO_H
#define GRACEFUL_TERMINATOR_GT_PROXY_INFO_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GT_TYPE_PROXY_INFO  gt_proxy_info_get_type ()

G_DECLARE_FINAL_TYPE (GtProxyInfo, gt_proxy_info, GT, PROXY_INFO, GObject)

GtProxyInfo* gt_proxy_info_get_default (void);
void gt_proxy_info_apply_to_environ (GtProxyInfo* self, char*** env);

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_PROXY_INFO_H
