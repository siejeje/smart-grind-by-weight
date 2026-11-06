#pragma once
#include <lvgl.h>
#include "../../config/constants.h"
#include "../../controllers/grind_mode.h"

class ReadyScreen {
private:
    lv_obj_t* screen;
    lv_obj_t* tabview;
    lv_obj_t* profile_tabs[4];
    lv_obj_t* weight_labels[3];
    lv_obj_t* menu_tab;
    lv_obj_t* profile_cards[3] = {nullptr, nullptr, nullptr};
    lv_obj_t* vertical_container = nullptr;   // vertical pager used in "Profile swipe" mode
    bool vertical_profiles = false;
    int vertical_profile_index = 0;
    bool visible;

public:
    void create();
    void show();
    void hide();
    void update_profile_values(const float values[3], GrindMode mode);
    void set_active_tab(int tab);
    // Profile-swipe layout: profiles stacked vertically on one page, MENU on the next page.
    void set_vertical_profiles(bool enabled, int active_tab);
    bool is_vertical_profiles() const { return vertical_profiles; }
    // Map the tabview's physical page index to the logical tab (0-2 profiles, 3 menu)
    int logical_tab_from_tabview(int physical) const;
    void set_profile_long_press_handler(lv_event_cb_t handler);
    
    bool is_visible() const { return visible; }
    lv_obj_t* get_screen() const { return screen; }
    lv_obj_t* get_tabview() const { return tabview; }
    lv_obj_t* get_menu_tab() const { return menu_tab; }
    
private:
    void create_profile_page(lv_obj_t* parent, int profile_index, const char* profile_name, float weight);
    void create_menu_page(lv_obj_t* parent);
    static void vertical_scroll_end_cb(lv_event_t* e);
};
