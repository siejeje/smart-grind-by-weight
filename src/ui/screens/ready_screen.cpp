#include "ready_screen.h"
#include <Arduino.h>
#include "../../config/constants.h"
#include "../../controllers/grind_mode_traits.h"
#include "../ui_helpers.h"

void ReadyScreen::create() {
    screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(screen, LV_PCT(100), UI_MAIN_AREA_HEIGHT);
    lv_obj_align(screen, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_add_flag(screen, LV_OBJ_FLAG_GESTURE_BUBBLE);

    // Create tabview
    tabview = lv_tabview_create(screen);
    lv_obj_set_size(tabview, LV_PCT(100), LV_PCT(100));
    lv_obj_align(tabview, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(tabview, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_add_flag(tabview, LV_OBJ_FLAG_GESTURE_BUBBLE);

    // Hide tab buttons for swipe-only interface
    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_add_flag(tab_btns, LV_OBJ_FLAG_HIDDEN);

    // Transparent background
    lv_obj_set_style_bg_opa(tabview, LV_OPA_TRANSP, 0);

    // Add profile tabs
    profile_tabs[0] = lv_tabview_add_tab(tabview, "Single");
    profile_tabs[1] = lv_tabview_add_tab(tabview, "Double");
    profile_tabs[2] = lv_tabview_add_tab(tabview, "Custom");
    menu_tab = lv_tabview_add_tab(tabview, "MENU");
    profile_tabs[3] = menu_tab;

    // Default weights
    float default_weights[3] = {USER_SINGLE_ESPRESSO_WEIGHT_G, USER_DOUBLE_ESPRESSO_WEIGHT_G, USER_CUSTOM_PROFILE_WEIGHT_G};
    const char* names[3] = {"SINGLE", "DOUBLE", "CUSTOM"};
    
    for (int i = 0; i < 3; i++) {
        // Each profile lives in a "card" that can be moved between the horizontal tab
        // pages and the vertical pager without rebuilding its widgets.
        lv_obj_t* card = lv_obj_create(profile_tabs[i]);
        lv_obj_remove_style_all(card);
        lv_obj_set_size(card, LV_PCT(100), LV_PCT(100));
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(card, LV_OBJ_FLAG_GESTURE_BUBBLE);
        lv_obj_add_flag(card, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
        lv_obj_add_flag(card, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
        profile_cards[i] = card;
        create_profile_page(card, i, names[i], default_weights[i]);
        lv_obj_clear_flag(profile_tabs[i], LV_OBJ_FLAG_SCROLLABLE);
    }

    // Vertical pager (hidden until "Profile swipe" is enabled)
    vertical_container = lv_obj_create(profile_tabs[0]);
    lv_obj_remove_style_all(vertical_container);
    lv_obj_set_size(vertical_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(vertical_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(vertical_container, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(vertical_container, LV_SCROLL_SNAP_CENTER);
    lv_obj_add_flag(vertical_container, LV_OBJ_FLAG_SCROLL_ONE);
    lv_obj_add_flag(vertical_container, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_clear_flag(vertical_container, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_clear_flag(vertical_container, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scrollbar_mode(vertical_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(vertical_container, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_event_cb(vertical_container, vertical_scroll_end_cb, LV_EVENT_SCROLL_END, this);
    lv_obj_add_flag(vertical_container, LV_OBJ_FLAG_HIDDEN);

    // Create menu tab page
    create_menu_page(menu_tab);

    update_profile_values(default_weights, GrindMode::WEIGHT);

    visible = false;
}

void ReadyScreen::create_profile_page(lv_obj_t* parent, int profile_index, const char* profile_name, float weight) {
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(parent, 0, 0);

    lv_obj_t* name_label;
    (void)create_profile_label(parent, &name_label, &weight_labels[profile_index]);
    lv_label_set_text(name_label, profile_name);
    lv_obj_add_flag(name_label, LV_OBJ_FLAG_CLICKABLE);
    
    char weight_text[16];
    snprintf(weight_text, sizeof(weight_text), SYS_WEIGHT_DISPLAY_FORMAT, weight);
    lv_label_set_text(weight_labels[profile_index], weight_text);
    lv_obj_add_flag(weight_labels[profile_index], LV_OBJ_FLAG_CLICKABLE);
}

void ReadyScreen::create_menu_page(lv_obj_t* parent) {
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(parent, UI_SY(20), 0);

    // Info label
    lv_obj_t* info_label = lv_label_create(parent);
    lv_label_set_text(info_label, "MAIN\nMENU");
    lv_obj_set_style_text_font(info_label, UI_FONT_32, 0);
    lv_obj_set_style_text_color(info_label, lv_color_hex(THEME_COLOR_TEXT_PRIMARY), 0);
    lv_obj_set_style_text_align(info_label, LV_TEXT_ALIGN_CENTER, 0);
}

void ReadyScreen::show() {
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_HIDDEN);
    visible = true;
}

void ReadyScreen::hide() {
    lv_obj_add_flag(screen, LV_OBJ_FLAG_HIDDEN);
    visible = false;
}

void ReadyScreen::update_profile_values(const float values[3], GrindMode mode) {
    for (int i = 0; i < 3; i++) {
        if (weight_labels[i]) {
            char text[24];
            format_ready_value(text, sizeof(text), mode, values[i]);
            lv_label_set_text(weight_labels[i], text);
        }
    }
}

void ReadyScreen::set_active_tab(int tab) {
    if (tab < 0 || tab >= 4) {
        return;
    }
    if (!vertical_profiles) {
        lv_tabview_set_act(tabview, tab, LV_ANIM_OFF);
        return;
    }
    if (tab < 3) {
        vertical_profile_index = tab;
        lv_tabview_set_act(tabview, 0, LV_ANIM_OFF);
        lv_obj_update_layout(vertical_container);
        lv_obj_scroll_to_y(vertical_container, tab * lv_obj_get_content_height(vertical_container), LV_ANIM_OFF);
    } else {
        lv_tabview_set_act(tabview, 1, LV_ANIM_OFF);  // MENU is the second visible page
    }
}

int ReadyScreen::logical_tab_from_tabview(int physical) const {
    if (!vertical_profiles) {
        return physical;
    }
    return (physical == 0) ? vertical_profile_index : 3;
}

void ReadyScreen::set_vertical_profiles(bool enabled, int active_tab) {
    if (enabled == vertical_profiles) {
        set_active_tab(active_tab);
        return;
    }
    vertical_profiles = enabled;
    if (enabled) {
        lv_obj_clear_flag(vertical_container, LV_OBJ_FLAG_HIDDEN);
        for (int i = 0; i < 3; i++) {
            lv_obj_set_parent(profile_cards[i], vertical_container);
        }
        // Hidden tab pages are skipped by the flex layout: pages become [profiles][MENU]
        lv_obj_add_flag(profile_tabs[1], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(profile_tabs[2], LV_OBJ_FLAG_HIDDEN);
    } else {
        for (int i = 0; i < 3; i++) {
            lv_obj_set_parent(profile_cards[i], profile_tabs[i]);
        }
        lv_obj_add_flag(vertical_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(profile_tabs[1], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(profile_tabs[2], LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_update_layout(tabview);
    set_active_tab(active_tab);
}

void ReadyScreen::vertical_scroll_end_cb(lv_event_t* e) {
    ReadyScreen* self = static_cast<ReadyScreen*>(lv_event_get_user_data(e));
    if (!self || !self->vertical_profiles) {
        return;
    }
    lv_obj_t* cont = self->vertical_container;
    int32_t h = lv_obj_get_content_height(cont);
    if (h <= 0) {
        return;
    }
    lv_point_t end;
    lv_obj_get_scroll_end(cont, &end);
    int idx = (end.y + h / 2) / h;
    if (idx < 0) idx = 0;
    if (idx > 2) idx = 2;
    if (idx != self->vertical_profile_index) {
        self->vertical_profile_index = idx;
        // Reuse the existing TAB_CHANGE pipeline (handler maps the index via logical_tab_from_tabview)
        lv_obj_send_event(self->tabview, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

void ReadyScreen::set_profile_long_press_handler(lv_event_cb_t handler) {
    for (int i = 0; i < 3; i++) {
        if (weight_labels[i]) {
            lv_obj_add_event_cb(weight_labels[i], handler, LV_EVENT_LONG_PRESSED, NULL);
        }
    }
}
