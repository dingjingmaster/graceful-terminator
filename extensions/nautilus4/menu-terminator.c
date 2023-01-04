//
// Created by dingjing on 1/4/23.
//

#include "menu-terminator.h"

struct _MenuTerminatorItemProvider
{
    GObject parent_instance;
};

//G_DECLARE_FINAL_TYPE(MenuTerminatorItemProvider, mt_menu_terminator_item_provider, MT, MT_TYPE_MENU_ITEM_PROVIDER, GObject);
static void mt_menu_terminator_item_provider_iface_init ();

static GType gMenuTerminatorItemProviderType = 0;

void mt_menu_terminator_item_provider_load (GTypeModule* module)
{
    const GTypeInfo info = {
        sizeof (GObjectClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) NULL,
        NULL,
        NULL,
        sizeof (GObject), 0,
        (GInstanceInitFunc) NULL
    };

    const GInterfaceInfo ifaceInfo = {
        .interface_init = (GInterfaceInitFunc) NULL,
        .interface_finalize = NULL,
        .interface_data = NULL,
    };

    gMenuTerminatorItemProviderType = g_type_module_register_type (module, G_TYPE_OBJECT, "GracefulTerminatorNautilusPlugin", &info, 0);

    g_type_module_add_interface (module, gMenuTerminatorItemProviderType, NAUTILUS_TYPE_)
}
