#pragma once
#include <Arduino.h>
#include <driver/rmt_tx.h>
#include <driver/rmt_encoder.h>
#include <esp_timer.h>
#include <functional>
#include "../config/constants.h"

// Forward declarations
struct GrindEventData;
enum class UIGrindEvent;

class Grinder {
private:
    int motor_pin;
    bool grinding;
    bool initialized;

    // RMT pulse control
    rmt_channel_handle_t rmt_channel;
    rmt_encoder_handle_t current_encoder;
    bool pulse_active;
    bool rmt_initialized;

    // Motor settling tracking
    unsigned long motor_start_time;

    // Background indicator state (always compiled in)
    bool background_active;
    std::function<void(const GrindEventData&)> ui_event_callback;

    void emit_background_change(bool active);

    // Independent hard limit for one continuous motor activation
    esp_timer_handle_t safety_timer = nullptr;
    volatile bool safety_tripped = false;
    void arm_safety_timer();
    void disarm_safety_timer();
    static void safety_timer_cb(void* arg);

public:
    void init(int pin);
    void start();
    void stop();
    
    // RMT-based precise pulse control
    void start_pulse_rmt(uint32_t duration_ms);
    bool is_pulse_complete();
    
    bool is_grinding() const { return grinding; }
    bool is_initialized() const { return initialized; }
    bool is_motor_settled() const;
    bool was_safety_tripped() const { return safety_tripped; }

    // Background indicator setup (always compiled in)
    void set_ui_event_callback(const std::function<void(const GrindEventData&)>& callback);
};
