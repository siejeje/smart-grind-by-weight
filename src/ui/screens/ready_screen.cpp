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

    // Vertical stack (used only in "Profile swipe" mode): plain, non-scrollable container
    // holding all three cards; vertical gestures slide the cards with an animation.
    vertical_container = lv_obj_create(profile_tabs[0]);
    lv_obj_remove_style_all(vertical_container);
    lv_obj_set_size(vertical_container, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(vertical_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(vertical_container, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_flag(vertical_container, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_add_flag(vertical_container, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
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
        lv_tabview_set_act(tabview, 0, LV_ANIM_OFF);
        place_vertical_cards(tab);
    } else {
        lv_tabview_set_act(tabview, 1, LV_ANIM_OFF);  // MENU is the second visible page
    }
}

void ReadyScreen::place_vertical_cards(int active) {
    for (int i = 0; i < 3; i++) lv_anim_delete(profile_cards[i], nullptr);
    vertical_profile_index = active;
    for (int i = 0; i < 3; i++) {
        lv_obj_set_pos(profile_cards[i], 0, 0);
        lv_obj_set_style_opa(profile_cards[i], LV_OPA_COVER, 0);
        if (i == active) {
            lv_obj_clear_flag(profile_cards[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(profile_cards[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void ReadyScreen::anim_y_cb(lv_anim_t* a, int32_t v) {
    lv_obj_t* obj = static_cast<lv_obj_t*>(a->var);
    lv_obj_set_y(obj, v);
    // Fade with distance from the centre so the moving card never covers the status bar
    int32_t span = LV_MAX(LV_ABS(a->start_value), LV_ABS(a->end_value));
    int32_t d = LV_ABS(v);
    lv_opa_t opa = span > 0 ? (lv_opa_t)LV_CLAMP(0, 255 - (d * 255 * 2) / span, 255) : LV_OPA_COVER;
    lv_obj_set_style_opa(obj, opa, 0);
}

void ReadyScreen::anim_hide_done_cb(lv_anim_t* a) {
    lv_obj_t* obj = static_cast<lv_obj_t*>(a->var);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_y(obj, 0);
    lv_obj_set_style_opa(obj, LV_OPA_COVER, 0);
}

void ReadyScreen::slide_vertical_profile(int next, bool up) {
    if (!vertical_profiles || next < 0 || next > 2) {
        return;
    }
    if (next == vertical_profile_index) {
        return;
    }
    // Finish any running slide immediately
    place_vertical_cards(vertical_profile_index);

    int32_t h = lv_obj_get_height(vertical_container);
    if (h <= 0) {
        place_vertical_cards(next);
        return;
    }
    lv_obj_t* out_card = profile_cards[vertical_profile_index];
    lv_obj_t* in_card = profile_cards[next];
    vertical_profile_index = next;

    const int32_t out_end = up ? -h : h;   // swipe up: current card leaves upwards
    const int32_t in_start = up ? h : -h;  // next card enters from below
    lv_obj_set_y(in_card, in_start);
    lv_obj_clear_flag(in_card, LV_OBJ_FLAG_HIDDEN);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_custom_exec_cb(&a, anim_y_cb);
    lv_anim_set_duration(&a, 220);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);

    lv_anim_set_var(&a, out_card);
    lv_anim_set_values(&a, 0, out_end);
    lv_anim_set_completed_cb(&a, anim_hide_done_cb);
    lv_anim_start(&a);

    lv_anim_set_var(&a, in_card);
    lv_anim_set_values(&a, in_start, 0);
    lv_anim_set_completed_cb(&a, nullptr);
    lv_anim_start(&a);
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
        place_vertical_cards((active_tab >= 0 && active_tab < 3) ? active_tab : vertical_profile_index);
    } else {
        for (int i = 0; i < 3; i++) lv_anim_delete(profile_cards[i], nullptr);
        for (int i = 0; i < 3; i++) {
            lv_obj_set_parent(profile_cards[i], profile_tabs[i]);
            lv_obj_set_pos(profile_cards[i], 0, 0);
            lv_obj_clear_flag(profile_cards[i], LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_add_flag(vertical_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(profile_tabs[1], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(profile_tabs[2], LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_update_layout(tabview);
    set_active_tab(active_tab);
}


void ReadyScreen::set_profile_long_press_handler(lv_event_cb_t handler) {
    for (int i = 0; i < 3; i++) {
        if (weight_labels[i]) {
            lv_obj_add_event_cb(weight_labels[i], handler, LV_EVENT_LONG_PRESSED, NULL);
        }
    }
}
