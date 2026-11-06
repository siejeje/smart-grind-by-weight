#pragma once
#include <lvgl.h>
#include <stdint.h>

class UIManager;

// Top status strip shown on every screen:
//  - left:  live scale weight (always visible once the scale is ready)
//  - right: battery % + charge icon (boards with HW_HAS_BATTERY_MONITOR),
//           diagnostic warning icon, Bluetooth icon

class StatusIndicatorController {
public:
    explicit StatusIndicatorController(UIManager* manager);

    void build();
    void update();

private:
    void update_ble_status_icon();
    void update_warning_icon();
    void update_weight_label();
    void update_battery();

    UIManager* ui_manager_;
    lv_obj_t* right_group_ = nullptr;
    lv_obj_t* ble_status_icon_ = nullptr;
    lv_obj_t* warning_icon_ = nullptr;
    lv_obj_t* battery_label_ = nullptr;
    lv_obj_t* weight_label_ = nullptr;

    uint32_t last_weight_update_ms_ = 0;
    int32_t last_weight_decigrams_ = INT32_MIN;
    uint32_t last_battery_update_ms_ = 0;
    float battery_voltage_ = 0.0f;
    int last_battery_percent_ = -1;
    bool last_charging_ = false;
};
