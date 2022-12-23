#include "gt-close-dialog.h"

#include <adwaita.h>
#include <glib/gi18n.h>

#include "gt-config.h"
#include "gt-process.h"

GtkWidget* gt_close_dialog_new (GtCloseDialogContext context, GPtrArray* commands)
{
    g_autoptr (GtkBuilder) builder = NULL;
    GtkWidget *dialog, *list;

    builder = gtk_builder_new_from_resource (GT_APPLICATION_PATH "gt-close-dialog.ui");

    dialog = GTK_WIDGET (gtk_builder_get_object (builder, "dialog"));
    list = GTK_WIDGET (gtk_builder_get_object (builder, "list"));

    switch (context) {
        case GT_CONTEXT_WINDOW: {
            g_object_set (dialog, "heading", _("Close Window?"), "body", _("Some commands are still running, closing this window will kill them and may lead to unexpected outcomes"), NULL);
            break;
        }
        case GT_CONTEXT_TAB: {
            g_object_set (dialog, "heading", _("Close Tab?"), "body", _("Some commands are still running, closing this tab will kill them and may lead to unexpected outcomes"), NULL);
            break;
        }
        default: {
            g_assert_not_reached ();
        }
    }

    for (int i = 0; i < commands->len; ++i) {
        GtProcess* process = g_ptr_array_index (commands, i);
        GtkWidget* row = g_object_new (ADW_TYPE_ACTION_ROW, "title", gt_process_get_exec (process), NULL);

        gtk_list_box_append (GTK_LIST_BOX (list), row);
    }

    return dialog;
}
