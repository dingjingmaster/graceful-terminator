//
// Created by dingjing on 23-5-24.
//
#include "nautilus-menu-provider.h"

#include <gio/gio.h>


struct _GTNautilusMenuProvider
{
    GObject     parent_instance;

    GList*      mItemList;
    char*       mLocation;
};

static void activate_terminator (NautilusMenuItem* self, gpointer udata);
static void gt_nautilus_menu_provider_iface_init (NautilusMenuProviderInterface* iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED(GTNautilusMenuProvider, gt_nautilus_menu_provider, G_TYPE_OBJECT, 0, G_IMPLEMENT_INTERFACE_DYNAMIC(NAUTILUS_TYPE_MENU_PROVIDER, gt_nautilus_menu_provider_iface_init))

GList* get_file_items (NautilusMenuProvider* provider G_GNUC_UNUSED, GList* files G_GNUC_UNUSED)
{
    for (GList* l = files; l; l = g_list_next(l)) {
        NautilusFileInfo* file = (NautilusFileInfo*) (l->data);
        g_autoptr(GFile) location = nautilus_file_info_get_location (file);
        g_autofree char* loc = g_file_get_path (location);
    }

    return NULL;
}

GList* get_background_items (NautilusMenuProvider* provider G_GNUC_UNUSED, NautilusFileInfo* currentFolder G_GNUC_UNUSED)
{
    GFile* location = nautilus_file_info_get_location (currentFolder); // no need free
    char* loc = g_file_get_path (location);

    GList* mItemList = NULL;
    NautilusMenuItem* menu = nautilus_menu_item_new ("Open in terminator", "Open in terminator", "Open in graceful-terminator", NULL);
    mItemList = g_list_append (mItemList, menu);
    g_signal_connect(menu, "activate", G_CALLBACK(activate_terminator), loc);

    return mItemList;
}

static void gt_nautilus_menu_provider_iface_init (NautilusMenuProviderInterface* iface)
{
    iface->get_file_items = get_file_items;
    iface->get_background_items = get_background_items;
}

void gt_nautilus_menu_provider_load(GTypeModule* module)
{
    gt_nautilus_menu_provider_register_type (module);
}

static void gt_nautilus_menu_provider_init (GTNautilusMenuProvider* self G_GNUC_UNUSED)
{
}

static void gt_nautilus_menu_provider_class_init (GTNautilusMenuProviderClass* kclass G_GNUC_UNUSED)
{

}

static void gt_nautilus_menu_provider_class_finalize (GTNautilusMenuProviderClass* kclass G_GNUC_UNUSED)
{

}


static void activate_terminator (NautilusMenuItem*, gpointer udata)
{
    g_autofree char* dir = udata ? g_strdup(udata) : g_strdup(g_get_home_dir());

    g_autofree char* cmd = g_strdup_printf("/usr/bin/graceful-terminator --working-directory='%s'", dir);

    system (cmd);
}