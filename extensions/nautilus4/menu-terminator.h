//
// Created by dingjing on 1/4/23.
//

#ifndef GRACEFUL_TERMINATOR_MENU_TERMINATOR_H
#define GRACEFUL_TERMINATOR_MENU_TERMINATOR_H
#include <glib-object.h>

typedef struct _MenuTerminatorItemProvider              MenuTerminatorItemProvider;
//typedef struct _MenuTerminatorItemProviderClass         MenuTerminatorItemProviderClass;

#define MT_TYPE_MENU_ITEM_PROVIDER                      (mt_menu_get_type())

G_DECLARE_FINAL_TYPE(MenuTerminatorItemProvider, mt_menu_terminator_item_provider, MT, MT_TYPE_MENU_ITEM_PROVIDER, GObject);

void mt_menu_terminator_item_provider_load (GTypeModule* module);


#endif //GRACEFUL_TERMINATOR_MENU_TERMINATOR_H
