//
// Created by dingjing on 23-5-24.
//

#include <nautilus-extension.h>

#include "nautilus-menu-provider.h"

void nautilus_module_initialize (GTypeModule* module)
{
//    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
//    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

    gt_nautilus_menu_provider_load(module);
}

void nautilus_module_shutdown (void)
{

}

void nautilus_module_list_types (const GType** types, int* numTypes)
{
    static GType typeList[1] = {0};

    g_assert(types != NULL);
    g_assert(numTypes != NULL);

    typeList[0] = GT_NAUTILUS_TYPE_MENU_PROVIDER;

    *types = typeList;
    *numTypes = G_N_ELEMENTS(typeList);
}