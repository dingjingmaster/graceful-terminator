//
// Created by dingjing on 23-5-24.
//

#ifndef GRACEFUL_TERMINATOR_NAUTILUS_MENU_PROVIDER_H
#define GRACEFUL_TERMINATOR_NAUTILUS_MENU_PROVIDER_H
#include <glib-object.h>
#include <nautilus-extension.h>

#define GT_NAUTILUS_TYPE_MENU_PROVIDER              (gt_nautilus_menu_provider_get_type())

G_DECLARE_FINAL_TYPE(GTNautilusMenuProvider, gt_nautilus_menu_provider, NAUTILUS, GT_MENU_PROVIDER, GObject)

void gt_nautilus_menu_provider_load(GTypeModule* module);

#endif //GRACEFUL_TERMINATOR_NAUTILUS_MENU_PROVIDER_H
