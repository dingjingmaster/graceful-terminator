//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_SETTINGS_H
#define GRACEFUL_TERMINATOR_GT_SETTINGS_H

#include <glib-object.h>
#include <pango/pango.h>

#include "gt-enums.h"

G_BEGIN_DECLS

typedef enum
{
    GT_THEME_AUTO = 0,   /*< nick=auto >*/
    GT_THEME_NIGHT = 1,  /*< nick=night >*/
    GT_THEME_DAY = 2,    /*< nick=day >*/
    GT_THEME_HACKER = 3, /*< nick=hacker >*/
} GtTheme;

#define GT_FONT_SCALE_MIN 0.5
#define GT_FONT_SCALE_MAX 4.0
#define GT_FONT_SCALE_DEFAULT 1.0


#define GT_TYPE_SETTINGS gt_settings_get_type ()

G_DECLARE_FINAL_TYPE (GtSettings, gt_settings, GT, SETTINGS, GObject)

PangoFontDescription* gt_settings_get_font          (GtSettings* self);
void                  gt_settings_increase_scale    (GtSettings* self);
void                  gt_settings_decrease_scale    (GtSettings* self);
void                  gt_settings_reset_scale       (GtSettings* self);
GStrv                 gt_settings_get_shell         (GtSettings* self);
void                  gt_settings_set_custom_shell  (GtSettings* self, const char* const* shell);
void                  gt_settings_set_scrollback    (GtSettings* self, int64_t value);
void                  gt_settings_get_size          (GtSettings* self, int* width, int* height);
void                  gt_settings_set_custom_size   (GtSettings* self, int width, int height);

G_END_DECLS


#endif //GRACEFUL_TERMINATOR_GT_SETTINGS_H
