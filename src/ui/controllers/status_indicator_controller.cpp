#include "status_indicator_controller.h"

#include <Arduino.h>
#include <math.h>
#include "../../config/constants.h"
#include "../../system/diagnostics_controller.h"
#include "../ui_manager.h"
#include "../../hardware/battery_monitor.h"

namespace {
constexpr uint32_t kWeightRefreshMs = 150;
constexpr uint32_t kBatteryRefreshMs = 1000;

#if defined(HW_HAS_BATTERY_MONITOR)
const char* battery_symbol(int pct) {
    if (pct >= 90) return LV_SYMBOL_BATTERY_FULL;
    if (pct >= 65) return LV_SYMBOL_BATTERY_3;
    if (pct >= 40) return LV_SYMBOL_BATTERY_2;
    if (pct >= 15) return LV_SYMBOL_BATTERY_1;
    return LV_SYMBOL_BATTERY_EMPTY;
}
#endif

lv_obj_t* make_status_label(lv_obj_t* parent, const lv_font_t* font, uint32_t color) {
    lv_obj_t* label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);
    return label;
}
}  // namespace

StatusIndicatorController::StatusIndicatorController(UIManager* manager)
    : ui_manager_(manager) {}

void StatusIndicatorController::build() {
    if (!ui_manager_ || right_group_) {
        return;
    }

    // Right-aligned row: [battery] [warning] [bluetooth]. Hidden items collapse.
    right_group_ = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(right_group_);
    lv_obj_set_size(right_group_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(right_group_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(right_group_, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(right_group_, UI_STATUS_ICON_SPACING_PX / 4, 0);
    lv_obj_align(right_group_, LV_ALIGN_TOP_RIGHT, -UI_STATUS_ICON_MARGIN_X_PX, UI_STATUS_ICON_MARGIN_Y_PX);
    lv_obj_clear_flag(right_group_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(right_group_, LV_OBJ_FLAG_SCROLLABLE);

#if defined(HW_HAS_BATTERY_MONITOR)
    BatteryMonitor::init();
    battery_label_ = make_status_label(right_group_, UI_FONT_16, THEME_COLOR_TEXT_PRIMARY);
    lv_label_set_text(battery_label_, "");
#endif

    warning_icon_ = make_status_label(right_group_, UI_STATUS_ICON_FONT, THEME_COLOR_WARNING);
    lv_label_set_text(warning_icon_, LV_SYMBOL_WARNING);
    lv_obj_add_flag(warning_icon_, LV_OBJ_FLAG_HIDDEN);

    ble_status_icon_ = make_status_label(right_group_, UI_STATUS_ICON_FONT, THEME_COLOR_ACCENT);
    lv_label_set_text(ble_status_icon_, LV_SYMBOL_BLUETOOTH);
    lv_obj_add_flag(ble_status_icon_, LV_OBJ_FLAG_HIDDEN);

    // Live weight, top-left
    weight_label_ = make_status_label(lv_scr_act(), UI_STATUS_ICON_FONT, THEME_COLOR_TEXT_PRIMARY);
    lv_label_set_text(weight_label_, "");
    lv_obj_align(weight_label_, LV_ALIGN_TOP_LEFT, UI_STATUS_ICON_MARGIN_X_PX, UI_STATUS_ICON_MARGIN_Y_PX);

    update();
}

void StatusIndicatorController::update() {
    update_ble_status_icon();
    update_warning_icon();
    update_weight_label();
    update_battery();
}

void StatusIndicatorController::update_weight_label() {
    if (!ui_manager_ || !weight_label_) {
        return;
    }
    uint32_t now = millis();
    if (now - last_weight_update_ms_ < kWeightRefreshMs) {
        return;
    }
    last_weight_update_ms_ = now;

    auto* hw = ui_manager_->hardware_manager;
    auto* sensor = hw ? hw->get_weight_sensor() : nullptr;
    // Note: is_data_ready() mirrors the HX711 DOUT line (toggles every sample) - don't use it here
    if (!sensor || !sensor->is_calibrated() || sensor->get_sample_count() <= 0) {
        if (last_weight_decigrams_ != INT32_MIN) {
            lv_label_set_text(weight_label_, "");
            last_weight_decigrams_ = INT32_MIN;
        }
        return;
    }

    float w = sensor->get_weight_high_latency();
    int32_t dg = static_cast<int32_t>(lroundf(w * 10.0f));
    if (dg == last_weight_decigrams_) {
        return;
    }
    last_weight_decigrams_ = dg;

    char buf[16];
    if (dg == 0) {
        snprintf(buf, sizeof(buf), "0.0g");  // avoid "-0.0g"
    } else {
        snprintf(buf, sizeof(buf), "%.1fg", dg / 10.0f);
    }
    lv_label_set_text(weight_label_, buf);
}

void StatusIndicatorController::update_battery() {
#if defined(HW_HAS_BATTERY_MONITOR)
    if (!battery_label_) {
        return;
    }
    uint32_t now = millis();
    if (last_battery_percent_ >= 0 && now - last_battery_update_ms_ < kBatteryRefreshMs) {
        return;
    }
    last_battery_update_ms_ = now;

    BatteryMonitor::update();
    battery_voltage_ = BatteryMonitor::voltage();
    bool charging = BatteryMonitor::charging();
    int pct = BatteryMonitor::percent();
    if (pct < 0) {
        return;
    }

    if (pct == last_battery_percent_ && charging == last_charging_) {
        return;
    }
    last_battery_percent_ = pct;
    last_charging_ = charging;

    char buf[32];
    if (charging) {
        snprintf(buf, sizeof(buf), LV_SYMBOL_CHARGE " %d%%", pct);
    } else {
        snprintf(buf, sizeof(buf), "%s %d%%", battery_symbol(pct), pct);
    }
    lv_label_set_text(battery_label_, buf);

    uint32_t color = THEME_COLOR_TEXT_PRIMARY;
    if (charging) color = THEME_COLOR_SUCCESS;
    else if (pct < 15) color = THEME_COLOR_ERROR;
    else if (pct < 30) color = THEME_COLOR_WARNING;
    lv_obj_set_style_text_color(battery_label_, lv_color_hex(color), 0);
#endif
}

void StatusIndicatorController::update_ble_status_icon() {
    if (!ui_manager_ || !ble_status_icon_) {
        return;
    }

    auto* bluetooth = ui_manager_->bluetooth_manager;
    if (bluetooth && bluetooth->is_enabled()) {
        lv_obj_clear_flag(ble_status_icon_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_text_color(ble_status_icon_,
                                    bluetooth->is_connected() ? lv_color_hex(THEME_COLOR_SUCCESS)
                                                              : lv_color_hex(THEME_COLOR_ACCENT),
                                    0);
    } else {
        lv_obj_add_flag(ble_status_icon_, LV_OBJ_FLAG_HIDDEN);
    }
}

void StatusIndicatorController::update_warning_icon() {
    if (!ui_manager_ || !warning_icon_) {
        return;
    }

    if (ui_manager_->diagnostics_controller_) {
        DiagnosticCode diagnostic = ui_manager_->diagnostics_controller_->get_highest_priority_warning();
        if (diagnostic != DiagnosticCode::NONE) {
            lv_obj_clear_flag(warning_icon_, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(warning_icon_, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        lv_obj_add_flag(warning_icon_, LV_OBJ_FLAG_HIDDEN);
    }
}
