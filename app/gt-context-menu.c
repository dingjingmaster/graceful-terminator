//
// Created by dingjing on 23-4-18.
//
#include "gt-context-menu.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib-object.h>

#include "gt-log.h"

struct _GtContextMenu
{
    GtkPopover      parent_instance;
};

G_DEFINE_TYPE(GtContextMenu, gt_context_menu, GTK_TYPE_POPOVER)

static void gt_context_menu_class_init (GtContextMenuClass* klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);


}

static void gt_context_menu_init (GtContextMenu* self)
{

}

gboolean ck (GtkWidget* w, GdkEvent* event, gpointer udata)
{
    LOG_DEBUG("ssssssssssssssssssssssss");
}

GtkWidget* gt_context_menu_new()
{

    g_autoptr(GMenu) menu = g_menu_new ();

    g_autoptr(GMenuItem) copy = g_menu_item_new (_("Copy"), NULL);
    g_menu_item_set_detailed_action (copy, "term.copy");
    g_menu_append_item (menu, copy);

    g_autoptr(GMenuItem) paste = g_menu_item_new (_("Paste"), NULL);
    g_menu_item_set_detailed_action (paste, "term.paste");
    g_menu_append_item (menu, paste);

    g_autoptr(GtkPopoverMenu) popupMenu = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));

    GtkWidget* selectAll = gtk_button_new_with_label(_("_Select All"));
    gboolean ret = gtk_popover_menu_add_child(GTK_POPOVER_MENU(popupMenu), selectAll, "context-menu");
    LOG_DEBUG("%s", ret ? "ok" : "error")

    g_signal_connect(GTK_WIDGET(popupMenu), "closed", (GCallback) ck, NULL);

    return g_object_ref(popupMenu);
}
