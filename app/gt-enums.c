//
// Created by dingjing on 12/22/22.
//
#include "gt-enums.h"

#include "gt-tab.h"
#include "gt-settings.h"
#include "gt-close-dialog.h"

#define C_ENUM(v)       ((gint) v)
#define C_FLAGS(v)      ((guint) v)

/* enumerations from "gt-close-dialog.h" */
GType gt_close_dialog_context_get_type (void)
{
    static gsize gtype_id = 0;
    static const GEnumValue values[] = {
        { C_ENUM(GT_CONTEXT_WINDOW), "GT_CONTEXT_WINDOW", "window" },
        { C_ENUM(GT_CONTEXT_TAB), "GT_CONTEXT_TAB", "tab" },
        { 0, NULL, NULL }
    };
    if (g_once_init_enter (&gtype_id)) {
        GType new_type = g_enum_register_static (g_intern_static_string ("GtCloseDialogContext"), values);
        g_once_init_leave (&gtype_id, new_type);
    }
    return (GType) gtype_id;
}

/* enumerations from "gt-settings.h" */
GType gt_theme_get_type (void)
{
    static gsize gtype_id = 0;
    static const GEnumValue values[] =
    {
        { C_ENUM(GT_THEME_AUTO), "GT_THEME_AUTO", "auto" },
        { C_ENUM(GT_THEME_NIGHT), "GT_THEME_NIGHT", "night" },
        { C_ENUM(GT_THEME_DAY), "GT_THEME_DAY", "day" },
        { C_ENUM(GT_THEME_HACKER), "GT_THEME_HACKER", "hacker" },
        { 0, NULL, NULL }
    };
    if (g_once_init_enter (&gtype_id)) {
        GType new_type = g_enum_register_static (g_intern_static_string ("GtTheme"), values);
        g_once_init_leave (&gtype_id, new_type);
    }
    return (GType) gtype_id;
}

/* enumerations from "gt-tab.h" */
GType gt_status_get_type (void)
{
    static gsize gtype_id = 0;
    static const GFlagsValue values[] =
    {
        { C_FLAGS(GT_NONE), "GT_NONE", "none" },
        { C_FLAGS(GT_REMOTE), "GT_REMOTE", "remote" },
        { C_FLAGS(GT_PRIVILEGED), "GT_PRIVILEGED", "privileged" },
        { 0, NULL, NULL }
    };
    if (g_once_init_enter (&gtype_id)) {
        GType new_type = g_flags_register_static (g_intern_static_string ("GtStatus"), values);
        g_once_init_leave (&gtype_id, new_type);
    }
    return (GType) gtype_id;
}

GType gt_zoom_get_type (void)
{
    static gsize gtype_id = 0;
    static const GEnumValue values[] =
    {
        { C_ENUM(GT_ZOOM_IN), "GT_ZOOM_IN", "in" },
        { C_ENUM(GT_ZOOM_OUT), "GT_ZOOM_OUT", "out" },
        { 0, NULL, NULL }
    };
    if (g_once_init_enter (&gtype_id)) {
        GType new_type = g_enum_register_static (g_intern_static_string ("GtZoom"), values);
        g_once_init_leave (&gtype_id, new_type);
    }
    return (GType) gtype_id;
}
