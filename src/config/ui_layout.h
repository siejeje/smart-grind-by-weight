#pragma once
//==============================================================================
// UI LAYOUT SCALING
//==============================================================================
// The UI was designed for the 280x456 AMOLED panel. Smaller panels remap the
// font sizes and scale fixed pixel dimensions so layouts fit without overlap.
// On the original AMOLED board every macro below is an identity mapping.
//
//   UI_FONT_<n>  - font that replaces montserrat <n> from the original design
//   UI_SX(v)     - horizontal dimension (width, x offset, horizontal padding)
//   UI_SY(v)     - vertical dimension (height, y offset, vertical padding/gap)
//   UI_TOP_INSET_PX - space reserved at the top for BLE/warning status icons
//==============================================================================
#include <lvgl.h>
#include "hardware.h"

#if defined(BOARD_WAVESHARE_ESP32S3_TOUCH_LCD_2)

#define UI_SX(v) ((int32_t)(((v) * 240 + 140) / 280))
#define UI_SY(v) ((int32_t)(((v) * 320 + 228) / 456))

#define UI_FONT_14 (&lv_font_montserrat_14)
#define UI_FONT_16 (&lv_font_montserrat_14)
#define UI_FONT_24 (&lv_font_montserrat_18)
#define UI_FONT_28 (&lv_font_montserrat_20)
#define UI_FONT_32 (&lv_font_montserrat_22)
#define UI_FONT_36 (&lv_font_montserrat_26)
#define UI_FONT_48 (&lv_font_montserrat_36)
#define UI_FONT_56 (&lv_font_montserrat_40)
#define UI_FONT_60 (&lv_font_montserrat_44)

#define UI_TOP_INSET_PX 22                 // status icon strip (icons are 18 px tall)
#define UI_MENU_TITLE_FONT (&lv_font_montserrat_22)
#define UI_AUTOTUNE_CONSOLE_RESERVE_PX 136
#define UI_STATUS_ICON_FONT (&lv_font_montserrat_18)
#define UI_STATUS_ICON_MARGIN_X_PX 6
#define UI_STATUS_ICON_MARGIN_Y_PX 2
#define UI_STATUS_ICON_SPACING_PX 30

#else

#define UI_SX(v) ((int32_t)(v))
#define UI_SY(v) ((int32_t)(v))

#define UI_FONT_14 (&lv_font_montserrat_14)
#define UI_FONT_16 (&lv_font_montserrat_16)
#define UI_FONT_24 (&lv_font_montserrat_24)
#define UI_FONT_28 (&lv_font_montserrat_28)
#define UI_FONT_32 (&lv_font_montserrat_32)
#define UI_FONT_36 (&lv_font_montserrat_36)
#define UI_FONT_48 (&lv_font_montserrat_48)
#define UI_FONT_56 (&lv_font_montserrat_56)
#define UI_FONT_60 (&lv_font_montserrat_60)

#define UI_TOP_INSET_PX 0
#define UI_MENU_TITLE_FONT (&lv_font_montserrat_36)
#define UI_AUTOTUNE_CONSOLE_RESERVE_PX 116
#define UI_STATUS_ICON_FONT (&lv_font_montserrat_24)
#define UI_STATUS_ICON_MARGIN_X_PX 10
#define UI_STATUS_ICON_MARGIN_Y_PX 10
#define UI_STATUS_ICON_SPACING_PX 45

#endif

// Height of the main content area on screens that share the display with the
// round grind button at the bottom (ready / grinding / OTA screens)
#if defined(BOARD_WAVESHARE_ESP32S3_TOUCH_LCD_2)
#define UI_MAIN_AREA_HEIGHT (HW_DISPLAY_HEIGHT_PX - HW_UI_BUTTON_SIZE_PX - 16)
#define UI_ARC_DIAMETER_PX 138
#define UI_ARC_WIDTH_PX 9
#define UI_GRIND_PAD_PX 4
#define UI_GRIND_GAP_PX 6
#define UI_CHART_HEIGHT_PX 100
// Calibration screen label offsets from the screen centre
#define UI_CAL_TITLE_Y -70
#define UI_CAL_INSTRUCTION_Y -16
#define UI_CAL_WEIGHT_Y 40
#define UI_CAL_NOISE_STATUS_Y 52
#define UI_CAL_NOISE_METRIC_Y 82
#else
#define UI_MAIN_AREA_HEIGHT LV_PCT(80)
#define UI_ARC_DIAMETER_PX THEME_PROGRESS_ARC_DIAMETER_PX
#define UI_ARC_WIDTH_PX 12
#define UI_GRIND_PAD_PX 20
#define UI_GRIND_GAP_PX 30
#define UI_CHART_HEIGHT_PX 140
#define UI_CAL_TITLE_Y -90
#define UI_CAL_INSTRUCTION_Y -20
#define UI_CAL_WEIGHT_Y 55
#define UI_CAL_NOISE_STATUS_Y 60
#define UI_CAL_NOISE_METRIC_Y 105
#endif

// Fonts used for the round grind/pulse button icons
#define UI_FONT_ICON_GRIND UI_FONT_24
#define UI_FONT_ICON_PULSE UI_FONT_32

// Top offset for full-height content screens so they clear the status strip
#ifndef UI_CONTENT_TOP_PX
#if UI_TOP_INSET_PX > 0
#define UI_CONTENT_TOP_PX (UI_TOP_INSET_PX - 4)
#else
#define UI_CONTENT_TOP_PX 0
#endif
#endif
