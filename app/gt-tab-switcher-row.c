#include "gt-config.h"
#include "gt-tab-switcher-row.h"


struct _GtTabSwitcherRow
{
    GtkListBoxRow parent_instance;

    GtkRevealer *revealer;
    GtkStack *icon_stack;
    GtkImage *icon;
    GtkSpinner *spinner;
    GtkLabel *title;
    GtkWidget *indicator_btn;
    GtkImage *indicator_icon;
    GtkWidget *close_btn;

    AdwTabPage *page;
    AdwTabView *view;
};


G_DEFINE_TYPE (GtTabSwitcherRow, gt_tab_switcher_row, GTK_TYPE_LIST_BOX_ROW)


enum
{
    PROP_0, PROP_PAGE, PROP_VIEW, LAST_PROP
};
static GParamSpec *pspecs[LAST_PROP] = {NULL,};


static inline void set_style_class(GtkWidget *widget, const char *style_class, gboolean enabled)
{
    GtkStyleContext *context = gtk_widget_get_style_context (widget);

    if (enabled) {
        gtk_style_context_add_class (context, style_class);
    } else {
        gtk_style_context_remove_class (context, style_class);
    }
}


static void update_pinned(GtTabSwitcherRow *self)
{
    set_style_class (GTK_WIDGET (self), "pinned", adw_tab_page_get_pinned (self->page));
}


static void update_icon(GtTabSwitcherRow *self)
{
    GIcon *gicon = adw_tab_page_get_icon (self->page);
    gboolean loading = adw_tab_page_get_loading (self->page);
    const char *name = loading ? "spinner" : "icon";

    if (!gicon) {
        gicon = adw_tab_view_get_default_icon (self->view);
    }

    gtk_image_set_from_gicon (self->icon, gicon);
    gtk_stack_set_visible_child_name (self->icon_stack, name);
}


static void update_spinner(GtTabSwitcherRow *self)
{
    gboolean loading = self->page && adw_tab_page_get_loading (self->page);
    gboolean mapped = gtk_widget_get_mapped (GTK_WIDGET (self));

    /* Don't use CPU when not needed */
    if (loading && mapped) {
        gtk_spinner_start (self->spinner);
    } else if (gtk_widget_get_realized (GTK_WIDGET (self))) {
        gtk_spinner_stop (self->spinner);
    }
}


static void update_loading(GtTabSwitcherRow *self)
{
    update_icon (self);
    update_spinner (self);
    set_style_class (GTK_WIDGET (self), "loading", adw_tab_page_get_loading (self->page));
}


static void update_indicator(GtTabSwitcherRow *self)
{
    GIcon *indicator = adw_tab_page_get_indicator_icon (self->page);
    gboolean activatable = adw_tab_page_get_indicator_activatable (self->page);

    gtk_image_set_from_gicon (self->indicator_icon, indicator);
    gtk_widget_set_visible (GTK_WIDGET (self->indicator_btn), indicator != NULL);
    gtk_widget_set_sensitive (GTK_WIDGET (self->indicator_btn), activatable);
}


static void update_needs_attention(GtTabSwitcherRow *self)
{
    set_style_class (GTK_WIDGET (self), "needs-attention", adw_tab_page_get_needs_attention (self->page));
}


static void indicator_clicked_cb(GtTabSwitcherRow *self)
{
    if (!self->page) {
        return;
    }

    g_signal_emit_by_name (self->view, "indicator-activated", self->page);
}


static void close_clicked_cb(GtTabSwitcherRow *self)
{
    if (!self->page) {
        return;
    }

    adw_tab_view_close_page (self->view, self->page);
}


static void destroy_cb(GtTabSwitcherRow *self)
{
    GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (self));

    g_assert (GTK_IS_LIST_BOX (parent));

    gtk_list_box_remove (GTK_LIST_BOX (parent), GTK_WIDGET (self));
}


static void gt_tab_switcher_row_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    GtTabSwitcherRow *self = GT_TAB_SWITCHER_ROW (object);

    switch (prop_id) {
        case PROP_PAGE:
            g_value_set_object (value, self->page);
            break;
        case PROP_VIEW:
            g_value_set_object (value, self->view);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void gt_tab_switcher_row_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    GtTabSwitcherRow *self = GT_TAB_SWITCHER_ROW (object);

    switch (prop_id) {
        case PROP_PAGE:
            self->page = g_value_get_object (value);
            break;
        case PROP_VIEW:
            self->view = g_value_get_object (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void gt_tab_switcher_row_constructed(GObject *object)
{
    GtTabSwitcherRow *self = GT_TAB_SWITCHER_ROW (object);

    G_OBJECT_CLASS (gt_tab_switcher_row_parent_class)->constructed (object);

    g_object_bind_property (self->page, "title", self->title, "label", G_BINDING_SYNC_CREATE);
    g_object_bind_property (self->page, "pinned", self->close_btn, "visible",
                            G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);

    g_signal_connect_object (self->page, "notify::pinned", G_CALLBACK (update_pinned), self, G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::icon", G_CALLBACK (update_icon), self, G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::loading", G_CALLBACK (update_loading), self, G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::indicator-icon", G_CALLBACK (update_indicator), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::indicator-activatable", G_CALLBACK (update_indicator), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::needs-attention", G_CALLBACK (update_needs_attention), self,
                             G_CONNECT_SWAPPED);

    g_signal_connect_object (self->view, "notify::default-icon", G_CALLBACK (update_icon), self, G_CONNECT_SWAPPED);

    update_pinned (self);
    update_loading (self);
    update_indicator (self);
    update_needs_attention (self);
}


static void gt_tab_switcher_row_map(GtkWidget *widget)
{
    GtTabSwitcherRow *self = GT_TAB_SWITCHER_ROW (widget);

    GTK_WIDGET_CLASS (gt_tab_switcher_row_parent_class)->map (widget);

    update_spinner (self);
}


static void gt_tab_switcher_row_unmap(GtkWidget *widget)
{
    GtTabSwitcherRow *self = GT_TAB_SWITCHER_ROW (widget);

    GTK_WIDGET_CLASS (gt_tab_switcher_row_parent_class)->unmap (widget);

    update_spinner (self);
}


static void gt_tab_switcher_row_class_init(GtTabSwitcherRowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->get_property = gt_tab_switcher_row_get_property;
    object_class->set_property = gt_tab_switcher_row_set_property;
    object_class->constructed = gt_tab_switcher_row_constructed;

    widget_class->map = gt_tab_switcher_row_map;
    widget_class->unmap = gt_tab_switcher_row_unmap;

    pspecs[PROP_PAGE] = g_param_spec_object ("page", "Page", "The page the row displays.", ADW_TYPE_TAB_PAGE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    pspecs[PROP_VIEW] = g_param_spec_object ("view", "View", "The view containing the page.", ADW_TYPE_TAB_VIEW,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    g_object_class_install_properties (object_class, LAST_PROP, pspecs);

    gtk_widget_class_set_template_from_resource (widget_class, GT_APPLICATION_PATH "gt-tab-switcher-row.ui");

    gtk_widget_class_bind_template_child (widget_class, GtTabSwitcherRow, revealer);
    gtk_widget_class_bind_template_child (widget_class, GtTabSwitcherRow, icon_stack);
    gtk_widget_class_bind_template_child (widget_class, GtTabSwitcherRow, icon);
    gtk_widget_class_bind_template_child (widget_class, GtTabSwitcherRow, spinner);
    gtk_widget_class_bind_template_child (widget_class, GtTabSwitcherRow, title);
    gtk_widget_class_bind_template_child (widget_class, GtTabSwitcherRow, indicator_btn);
    gtk_widget_class_bind_template_child (widget_class, GtTabSwitcherRow, indicator_icon);
    gtk_widget_class_bind_template_child (widget_class, GtTabSwitcherRow, close_btn);

    gtk_widget_class_bind_template_callback (widget_class, indicator_clicked_cb);
    gtk_widget_class_bind_template_callback (widget_class, close_clicked_cb);
}


static void gt_tab_switcher_row_init(GtTabSwitcherRow *self)
{
    gtk_widget_init_template (GTK_WIDGET (self));
}


GtkWidget *gt_tab_switcher_row_new(AdwTabPage *page, AdwTabView *view)
{
    g_return_val_if_fail (ADW_IS_TAB_PAGE (page), NULL);
    g_return_val_if_fail (ADW_IS_TAB_VIEW (view), NULL);

    return g_object_new (GT_TYPE_TAB_SWITCHER_ROW, "page", page, "view", view, NULL);
}


AdwTabPage *gt_tab_switcher_row_get_page(GtTabSwitcherRow *self)
{
    g_return_val_if_fail (GT_IS_TAB_SWITCHER_ROW (self), NULL);

    return self->page;
}


void gt_tab_switcher_row_animate_close(GtTabSwitcherRow *self)
{
    g_return_if_fail (GT_IS_TAB_SWITCHER_ROW (self));

    if (!self->page) {
        return;
    }

    g_signal_handlers_disconnect_by_func (self->view, G_CALLBACK (update_icon), self);

    g_signal_handlers_disconnect_by_func (self->page, G_CALLBACK (update_pinned), self);
    g_signal_handlers_disconnect_by_func (self->page, G_CALLBACK (update_icon), self);
    g_signal_handlers_disconnect_by_func (self->page, G_CALLBACK (update_loading), self);
    g_signal_handlers_disconnect_by_func (self->page, G_CALLBACK (update_indicator), self);
    g_signal_handlers_disconnect_by_func (self->page, G_CALLBACK (update_needs_attention), self);

    self->page = NULL;

    g_signal_connect_swapped (self->revealer, "notify::child-revealed", G_CALLBACK (destroy_cb), self);

    gtk_revealer_set_reveal_child (self->revealer, FALSE);
}


void gt_tab_switcher_row_animate_open(GtTabSwitcherRow *self)
{
    g_return_if_fail (GT_IS_TAB_SWITCHER_ROW (self));

    if (!self->page) {
        return;
    }

    gtk_widget_show (GTK_WIDGET (self));
    gtk_revealer_set_reveal_child (self->revealer, TRUE);
}


gboolean gt_tab_switcher_row_is_animating(GtTabSwitcherRow *self)
{
    g_return_val_if_fail (GT_IS_TAB_SWITCHER_ROW (self), FALSE);

    return self->page == NULL;
}
