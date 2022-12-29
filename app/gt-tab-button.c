#include "gt-tab-button.h"

#include "gt-config.h"

#define XFT_DPI_MULTIPLIER (96.0 * PANGO_SCALE)

struct _GtTabButton
{
    GtkButton parent_instance;

    GtkLabel *label;
    GtkImage *icon;

    AdwTabView *view;
};


G_DEFINE_TYPE (GtTabButton, gt_tab_button, GTK_TYPE_BUTTON)


enum
{
    PROP_0, PROP_VIEW, LAST_PROP
};
static GParamSpec *pspecs[LAST_PROP] = {NULL,};


/* FIXME: I hope there is a better way to prevent the label from changing scale */
static void update_label_scale(GtTabButton *self, GtkSettings *settings)
{
    int xft_dpi;
    PangoAttrList *attrs;
    PangoAttribute *scale_attribute;

    g_object_get (settings, "gtk-xft-dpi", &xft_dpi, NULL);

    attrs = pango_attr_list_new ();

    scale_attribute = pango_attr_scale_new (XFT_DPI_MULTIPLIER / (double) xft_dpi);

    pango_attr_list_change (attrs, scale_attribute);

    gtk_label_set_attributes (self->label, attrs);

    pango_attr_list_unref (attrs);
}


static void xft_dpi_changed(GtTabButton *self, GParamSpec *pspec, GtkSettings *settings)
{
    update_label_scale (self, settings);
}


static void update_icon(GtTabButton *self)
{
    gboolean display_label = FALSE;
    gboolean small_label = FALSE;
    const char *icon_name = "tab-counter-symbolic";
    g_autofree char *label_text = NULL;
    GtkStyleContext *context;

    if (self->view) {
        guint n_pages = adw_tab_view_get_n_pages (self->view);

        small_label = n_pages >= 10;

        if (n_pages < 100) {
            display_label = TRUE;
            label_text = g_strdup_printf ("%u", n_pages);
        } else {
            icon_name = "tab-overflow-symbolic";
        }
    }

    context = gtk_widget_get_style_context (GTK_WIDGET (self->label));

    if (small_label) {
        gtk_style_context_add_class (context, "small");
    } else {
        gtk_style_context_remove_class (context, "small");
    }

    gtk_widget_set_visible (GTK_WIDGET (self->label), display_label);
    gtk_label_set_text (self->label, label_text);
    gtk_image_set_from_icon_name (self->icon, icon_name);
}


static void gt_tab_button_dispose(GObject *object) 
{
    GtTabButton *self = GT_TAB_BUTTON (object);

    gt_tab_button_set_view (self, NULL);

    G_OBJECT_CLASS (gt_tab_button_parent_class)->dispose (object);
}


static void gt_tab_button_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    GtTabButton *self = GT_TAB_BUTTON (object);

    switch (prop_id) {
        case PROP_VIEW:
            g_value_set_object (value, gt_tab_button_get_view (self));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void gt_tab_button_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    GtTabButton *self = GT_TAB_BUTTON (object);

    switch (prop_id) {
        case PROP_VIEW:
            gt_tab_button_set_view (self, g_value_get_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void gt_tab_button_class_init(GtTabButtonClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->dispose = gt_tab_button_dispose;
    object_class->get_property = gt_tab_button_get_property;
    object_class->set_property = gt_tab_button_set_property;

    pspecs[PROP_VIEW] = g_param_spec_object ("view", "View", "The view the tab button displays.", ADW_TYPE_TAB_VIEW,
                                             G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, LAST_PROP, pspecs);

    gtk_widget_class_set_template_from_resource (widget_class, GT_APPLICATION_PATH "gt-tab-button.ui");

    gtk_widget_class_bind_template_child (widget_class, GtTabButton, label);
    gtk_widget_class_bind_template_child (widget_class, GtTabButton, icon);
}


static void gt_tab_button_init(GtTabButton *self) 
{
    GtkSettings *settings;

    gtk_widget_init_template (GTK_WIDGET (self));

    update_icon (self);

    settings = gtk_widget_get_settings (GTK_WIDGET (self));

    update_label_scale (self, settings);
    g_signal_connect_object (settings, "notify::gtk-xft-dpi", G_CALLBACK (xft_dpi_changed), self, G_CONNECT_SWAPPED);
}

GtkWidget *gt_tab_button_new(void) 
{
    return g_object_new (KGX_TYPE_TAB_BUTTON, NULL);
}


AdwTabView *gt_tab_button_get_view(GtTabButton *self)
{
    g_return_val_if_fail (GT_IS_TAB_BUTTON (self), NULL);

    return self->view;
}


void gt_tab_button_set_view(GtTabButton *self, AdwTabView *view)
{
    g_return_if_fail (GT_IS_TAB_BUTTON (self));
    g_return_if_fail (view == NULL || ADW_IS_TAB_VIEW (view));

    if (self->view == view) {
        return;
    }

    if (self->view) {
        g_signal_handlers_disconnect_by_func (self->view, update_icon, self);
    }

    g_set_object (&self->view, view);

    if (self->view) {
        g_signal_connect_object (self->view, "notify::n-pages", G_CALLBACK (update_icon), self, G_CONNECT_SWAPPED);
    }

    update_icon (self);

    g_object_notify_by_pspec (G_OBJECT (self), pspecs[PROP_VIEW]);
}
