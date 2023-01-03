//
// Created by dingjing on 1/3/23.
//

#define  GETTEXT_PACKAGE = PROJECT_NAME"-nautilus-menu"

#include <glib/gi18n-lib.h>
#include <nautilus-extension.h>

void nautilus_module_initialize (GTypeModule* module)
{
//    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (PACKAGE_NAME, "UTF-8");

    //
}

void nautilus_module_shutdown (void)
{

}

void nautilus_module_list_types (const GType** types, int* numTypes)
{
    static GType typeList[1] = {0};

    g_assert (types != NULL);
    g_assert (numTypes != NULL);

    typeList[0] = NAUTILUS_TYPE_MENU_ITEM;

    *types = typeList;
    *numTypes = G_N_ELEMENTS(typeList);
}