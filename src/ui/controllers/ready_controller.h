#pragma once
#include <lvgl.h>
#include "../event_bridge_lvgl.h"

class UIManager;

// Handles profile tab navigation, long-press editing, and swipe mode switching

class ReadyUIController {
public:
    explicit ReadyUIController(UIManager* manager);

    void register_events();
    void update();
    void refresh_profiles();
    void handle_tab_change(int tab);
    void handle_profile_long_press();
    void toggle_mode();
    void handle_vertical_swipe(bool up);
    void apply_swipe_layout();  // re-read swipe prefs and switch ready-screen layout

private:
    UIManager* ui_manager_;
};
