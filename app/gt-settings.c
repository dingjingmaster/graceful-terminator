#include "gt-config.h"

#include <gio/gio.h>
#include <vte/vte.h>

#include "gt-log.h"
#include "gt-settings.h"


#define DESKTOP_INTERFACE_SETTINGS_SCHEMA   "org.gnome.desktop.interface"
#define MONOSPACE_FONT_KEY_NAME             "monospace-font-name"
#define RESTORE_SIZE_KEY                    "restore-window-size"
#define LAST_SIZE_KEY                       "last-window-size"

struct _GtSettings
{
    GObject parent_instance;

    GtTheme theme;
    double scale;
    int64_t scrollback_lines;

    GSettings *settings;
    GSettings *desktop_interface;
};


G_DEFINE_TYPE (GtSettings, gt_settings, G_TYPE_OBJECT)


enum
{
    PROP_0,
    PROP_THEME,
    PROP_FONT,
    PROP_FONT_SCALE,
    PROP_SCALE_CAN_INCREASE,
    PROP_SCALE_CAN_DECREASE,
    PROP_SCROLLBACK_LINES,
    LAST_PROP
};

static GParamSpec *pspecs[LAST_PROP] = {NULL,};


static void gt_settings_dispose(GObject *object)
{
    GtSettings *self = GT_SETTINGS (object);

    g_clear_object (&self->settings);
    g_clear_object (&self->desktop_interface);

    G_OBJECT_CLASS (gt_settings_parent_class)->dispose (object);
}


static void update_scale(GtSettings *self, double value)
{
    double clamped = CLAMP (value, GT_FONT_SCALE_MIN, GT_FONT_SCALE_MAX);

    if (self->scale == clamped) {
        return;
    }

    self->scale = clamped;

    g_object_notify_by_pspec (G_OBJECT (self), pspecs[PROP_FONT_SCALE]);
    g_object_notify_by_pspec (G_OBJECT (self), pspecs[PROP_SCALE_CAN_INCREASE]);
    g_object_notify_by_pspec (G_OBJECT (self), pspecs[PROP_SCALE_CAN_DECREASE]);
}


static void gt_settings_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    GtSettings *self = GT_SETTINGS (object);

    switch (property_id) {
        case PROP_THEME:
            self->theme = g_value_get_enum (value);
            break;
        case PROP_FONT_SCALE:
            update_scale (self, g_value_get_double (value));
            break;
        case PROP_SCROLLBACK_LINES:
            gt_settings_set_scrollback (self, g_value_get_int64 (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static void gt_settings_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
    GtSettings *self = GT_SETTINGS (object);

    switch (property_id) {
        case PROP_THEME:
            g_value_set_enum (value, self->theme);
            break;
        case PROP_FONT:
            g_value_take_boxed (value, gt_settings_get_font (self));
            break;
        case PROP_FONT_SCALE:
            g_value_set_double (value, self->scale);
            break;
        case PROP_SCALE_CAN_INCREASE:
            g_value_set_boolean (value, self->scale < GT_FONT_SCALE_MAX);
            break;
        case PROP_SCALE_CAN_DECREASE:
            g_value_set_boolean (value, self->scale > GT_FONT_SCALE_MIN);
            break;
        case PROP_SCROLLBACK_LINES:
            g_value_set_int64 (value, self->scrollback_lines);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static void gt_settings_class_init(GtSettingsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = gt_settings_dispose;
    object_class->set_property = gt_settings_set_property;
    object_class->get_property = gt_settings_get_property;

    pspecs[PROP_THEME] = g_param_spec_enum ("theme", "Theme", "Terminal theme", GT_TYPE_THEME, GT_THEME_NIGHT,
                                            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    pspecs[PROP_FONT] = g_param_spec_boxed ("font", "Font", "Monospace font", PANGO_TYPE_FONT_DESCRIPTION,
                                            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

    pspecs[PROP_FONT_SCALE] = g_param_spec_double ("font-scale", "Font scale", "Font scaling", GT_FONT_SCALE_MIN,
                                                   GT_FONT_SCALE_MAX, GT_FONT_SCALE_DEFAULT,
                                                   G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
                                                   G_PARAM_EXPLICIT_NOTIFY);

    pspecs[PROP_SCALE_CAN_INCREASE] = g_param_spec_boolean ("scale-can-increase", NULL, NULL, TRUE,
                                                            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS |
                                                            G_PARAM_EXPLICIT_NOTIFY);

    pspecs[PROP_SCALE_CAN_DECREASE] = g_param_spec_boolean ("scale-can-decrease", NULL, NULL, TRUE,
                                                            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS |
                                                            G_PARAM_EXPLICIT_NOTIFY);

    pspecs[PROP_SCROLLBACK_LINES] = g_param_spec_int64 ("scrollback-lines", "Scrollback Lines",
                                                        "Size of the scrollback", G_MININT64, G_MAXINT64, 512,
                                                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
                                                        G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, LAST_PROP, pspecs);
}


static void font_changed(GSettings *settings, const char *key, GtSettings *self)
{
    g_object_notify_by_pspec (G_OBJECT (self), pspecs[PROP_FONT]);
}


static void restore_window_size_changed(GSettings *settings, const char *key, GtSettings *self)
{
    if (!g_settings_get_boolean (self->settings, RESTORE_SIZE_KEY)) {
        g_settings_set (self->settings, LAST_SIZE_KEY, "(ii)", -1, -1);
    }
}


static void gt_settings_init(GtSettings *self)
{
    self->settings = g_settings_new (GT_APPLICATION_ID);
    g_settings_bind (self->settings, "theme", self, "theme", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind (self->settings, "font-scale", self, "font-scale", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind (self->settings, "scrollback-lines", self, "scrollback-lines", G_SETTINGS_BIND_DEFAULT);

    g_signal_connect (self->settings, "changed::restore-window-size", G_CALLBACK (restore_window_size_changed), self);

    self->desktop_interface = g_settings_new (DESKTOP_INTERFACE_SETTINGS_SCHEMA);
    g_signal_connect (self->desktop_interface, "changed::" MONOSPACE_FONT_KEY_NAME, G_CALLBACK (font_changed), self);
}


PangoFontDescription *gt_settings_get_font(GtSettings *self)
{
    g_autofree char *font = NULL;

    g_return_val_if_fail (GT_IS_SETTINGS (self), NULL);

    font = g_settings_get_string (self->desktop_interface, MONOSPACE_FONT_KEY_NAME);

    return pango_font_description_from_string (font);
}


void gt_settings_increase_scale(GtSettings *self)
{
    g_return_if_fail (GT_IS_SETTINGS (self));

    update_scale (self, self->scale + 0.1);
}


void gt_settings_decrease_scale(GtSettings *self)
{
    g_return_if_fail (GT_IS_SETTINGS (self));

    update_scale (self, self->scale - 0.1);
}


void gt_settings_reset_scale(GtSettings *self)
{
    g_return_if_fail (GT_IS_SETTINGS (self));

    update_scale (self, GT_FONT_SCALE_DEFAULT);
}


GStrv gt_settings_get_shell(GtSettings *self)
{
    g_autofree char *user_shell = NULL;
    g_auto (GStrv) shell = NULL;
    g_auto (GStrv) custom_shell = NULL;

    g_return_val_if_fail (GT_IS_SETTINGS (self), NULL);

    custom_shell = g_settings_get_strv (self->settings, "shell");

    if (g_strv_length (custom_shell) > 0) {
        return g_steal_pointer (&custom_shell);
    }

    user_shell = vte_get_user_shell ();

    if (G_LIKELY (user_shell)) {
        shell = g_new0 (char *, 2);
        shell[0] = g_steal_pointer (&user_shell);
        shell[1] = NULL;

        return g_steal_pointer (&shell);
    }

    /* We could probably do something other than /bin/sh */
    shell = g_new0 (char *, 2);
    shell[0] = g_strdup ("/bin/sh");
    shell[1] = NULL;
    LOG_DEBUG("No Shell! Defaulting to “%s”", shell[0]);

    return g_steal_pointer (&shell);
}


void gt_settings_set_custom_shell(GtSettings *self, const char *const *shell)
{
    g_return_if_fail (GT_IS_SETTINGS (self));

    g_settings_set_strv (self->settings, "shell", shell);
}


void gt_settings_set_scrollback(GtSettings *self, int64_t value)
{
    g_return_if_fail (GT_IS_SETTINGS (self));

    if (self->scrollback_lines == value) {
        return;
    }

    self->scrollback_lines = value;

    g_object_notify_by_pspec (G_OBJECT (self), pspecs[PROP_SCROLLBACK_LINES]);
}


void gt_settings_get_size(GtSettings *self, int *width, int *height)
{
    g_return_if_fail (GT_IS_SETTINGS (self));
    g_return_if_fail (width != NULL && height != NULL);

    if (!g_settings_get_boolean (self->settings, RESTORE_SIZE_KEY)) {
        *width = -1;
        *height = -1;
        return;
    }

    g_settings_get (self->settings, LAST_SIZE_KEY, "(ii)", width, height);
}


void gt_settings_set_custom_size(GtSettings *self, int width, int height)
{
    g_return_if_fail (GT_IS_SETTINGS (self));

    if (!g_settings_get_boolean (self->settings, RESTORE_SIZE_KEY)) {
        return;
    }

    g_debug ("Store window size: %i×%i", width, height);

    g_settings_set (self->settings, LAST_SIZE_KEY, "(ii)", width, height);
}
