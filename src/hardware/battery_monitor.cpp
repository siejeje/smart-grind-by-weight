#include "battery_monitor.h"

#include <Arduino.h>
#include <math.h>
#include "../config/hardware.h"

#ifndef HW_BATTERY_VOLTAGE_CAL
#define HW_BATTERY_VOLTAGE_CAL 1.0f
#endif

namespace {
#if defined(HW_HAS_BATTERY_MONITOR)
constexpr uint32_t kMeasureIntervalMs = 1000;
constexpr int kSamples = 16;

// Resting Li-ion (INR18650-25R) voltage -> approximate state of charge
struct SocPoint { float v; int pct; };
constexpr SocPoint kSocTable[] = {
    {4.15f, 100}, {4.10f, 95}, {4.00f, 84}, {3.90f, 70}, {3.80f, 55},
    {3.70f, 38},  {3.60f, 20}, {3.50f, 10}, {3.40f, 5},  {3.30f, 2}, {3.20f, 0},
};

bool g_init = false;
uint32_t g_last_ms = 0;
float g_filtered = 0.0f;
float g_raw = 0.0f;
int g_adc_mv = 0;
int g_percent = -1;
bool g_charging = false;

// --- "USB without battery" detection ---
// Without a cell the ETA6098 output is just a capacitor: it terminates, sags, restarts,
// so CHG_DET toggles and VBAT swings or floats above the normal 4.2 V regulation.
constexpr uint32_t kWindowMs = 12000;
constexpr int kToggleThreshold = 3;          // >=3 CHG edges in the window (plug/unplug = 1 edge each)
constexpr float kSpreadThreshold = 0.12f;    // VBAT swing [V] in the window while USB present
constexpr float kOverVoltage = 4.25f;        // a real Li-ion cell never sits above this
bool g_chg_last_pin = false;
bool g_chg_pin_init = false;
uint32_t g_edge_ms[8] = {0};
int g_edge_head = 0;
int g_edge_count = 0;
float g_win_min = 99.0f, g_win_max = 0.0f, g_prev_min = 99.0f, g_prev_max = 0.0f;
uint32_t g_win_start = 0;
bool g_usb_only = false;
uint32_t g_usb_only_until = 0;

int edges_in_window(uint32_t now) {
    int n = 0;
    for (int i = 0; i < g_edge_count && i < 8; ++i) {
        if (now - g_edge_ms[i] <= kWindowMs) n++;
    }
    return n;
}

void poll_chg_pin(uint32_t now) {
    bool pin = digitalRead(HW_CHARGE_DETECT_PIN) == HIGH;
    if (!g_chg_pin_init) {
        g_chg_pin_init = true;
        g_chg_last_pin = pin;
        return;
    }
    if (pin != g_chg_last_pin) {
        g_chg_last_pin = pin;
        g_edge_ms[g_edge_head] = now;
        g_edge_head = (g_edge_head + 1) % 8;
        if (g_edge_count < 8) g_edge_count++;
    }
}

float window_spread() {
    float mn = g_win_min < g_prev_min ? g_win_min : g_prev_min;
    float mx = g_win_max > g_prev_max ? g_win_max : g_prev_max;
    return (mx > mn) ? (mx - mn) : 0.0f;
}

int voltage_to_percent(float v) {
    if (v >= kSocTable[0].v) return 100;
    constexpr int n = sizeof(kSocTable) / sizeof(kSocTable[0]);
    for (int i = 1; i < n; ++i) {
        if (v >= kSocTable[i].v) {
            const SocPoint& hi = kSocTable[i - 1];
            const SocPoint& lo = kSocTable[i];
            float t = (v - lo.v) / (hi.v - lo.v);
            return lo.pct + static_cast<int>(lroundf(t * (hi.pct - lo.pct)));
        }
    }
    return 0;
}

int read_adc_mv() {
    // 16 samples, drop min and max, average the rest
    int mn = INT32_MAX, mx = INT32_MIN;
    int32_t sum = 0;
    for (int i = 0; i < kSamples; ++i) {
        int mv = analogReadMilliVolts(HW_BATTERY_ADC_PIN);
        sum += mv;
        if (mv < mn) mn = mv;
        if (mv > mx) mx = mv;
    }
    return static_cast<int>((sum - mn - mx) / (kSamples - 2));
}

void measure() {
    g_adc_mv = read_adc_mv();
    g_raw = g_adc_mv * HW_BATTERY_DIVIDER_RATIO * HW_BATTERY_VOLTAGE_CAL / 1000.0f;
    uint32_t now = millis();
    bool chg = digitalRead(HW_CHARGE_DETECT_PIN) == HIGH;

    // Sliding min/max of the raw voltage (two half-windows)
    if (now - g_win_start > kWindowMs / 2) {
        g_prev_min = g_win_min; g_prev_max = g_win_max;
        g_win_min = 99.0f; g_win_max = 0.0f;
        g_win_start = now;
    }
    if (g_raw < g_win_min) g_win_min = g_raw;
    if (g_raw > g_win_max) g_win_max = g_raw;

    int edges = edges_in_window(now);
    // Swing criterion only with a steady CHG line, so plugging the charger in (one edge and a
    // ~0.15 V step on a real cell) is not mistaken for a missing battery.
    bool no_cell = (edges >= kToggleThreshold) ||
                   (g_raw > kOverVoltage) ||
                   (chg && edges == 0 && window_spread() > kSpreadThreshold && g_win_max > 3.9f);
    if (no_cell) {
        g_usb_only = true;
        g_usb_only_until = now + kWindowMs;   // hold the state for a full window
    } else if (g_usb_only && (int32_t)(now - g_usb_only_until) > 0) {
        g_usb_only = false;
        g_filtered = 0.0f;                    // restart filter when a cell comes back
    }
    if (g_usb_only) {
        g_charging = chg;
        g_percent = -1;
        return;
    }

    if (g_filtered <= 0.0f || chg != g_charging) {
        // First reading or charger plugged/unplugged: the terminal voltage jumps,
        // restart the filter from the fresh value instead of slowly sliding.
        g_filtered = g_raw;
        g_percent = -1;
    } else {
        g_filtered = g_filtered * 0.9f + g_raw * 0.1f;
    }
    g_charging = chg;

    int pct = voltage_to_percent(g_filtered);
    if (g_percent < 0) {
        g_percent = pct;
    } else if (g_charging) {
        if (pct > g_percent) g_percent = pct;          // only rises while charging
    } else {
        if (pct <= g_percent - 2 || pct == 0) g_percent = pct;  // only falls on battery (2% hysteresis)
    }
}
#endif
}  // namespace

namespace BatteryMonitor {
void init() {
#if defined(HW_HAS_BATTERY_MONITOR)
    if (g_init) return;
    pinMode(HW_CHARGE_DETECT_PIN, INPUT);
    analogSetPinAttenuation(HW_BATTERY_ADC_PIN, ADC_11db);
    g_init = true;
    measure();
    g_last_ms = millis();
#endif
}

void update() {
#if defined(HW_HAS_BATTERY_MONITOR)
    if (!g_init) init();
    uint32_t now = millis();
    poll_chg_pin(now);   // cheap, called every UI loop to catch short CHG pulses
    if (now - g_last_ms < kMeasureIntervalMs) return;
    g_last_ms = now;
    measure();
#endif
}

bool available() {
#if defined(HW_HAS_BATTERY_MONITOR)
    return g_init;
#else
    return false;
#endif
}

float voltage() {
#if defined(HW_HAS_BATTERY_MONITOR)
    return g_filtered;
#else
    return 0.0f;
#endif
}
float raw_voltage() {
#if defined(HW_HAS_BATTERY_MONITOR)
    return g_raw;
#else
    return 0.0f;
#endif
}
int adc_millivolts() {
#if defined(HW_HAS_BATTERY_MONITOR)
    return g_adc_mv;
#else
    return 0;
#endif
}
int percent() {
#if defined(HW_HAS_BATTERY_MONITOR)
    return g_percent;
#else
    return -1;
#endif
}
bool usb_only() {
#if defined(HW_HAS_BATTERY_MONITOR)
    return g_usb_only;
#else
    return false;
#endif
}
int chg_toggles() {
#if defined(HW_HAS_BATTERY_MONITOR)
    return edges_in_window(millis());
#else
    return 0;
#endif
}
float vbat_spread() {
#if defined(HW_HAS_BATTERY_MONITOR)
    return window_spread();
#else
    return 0.0f;
#endif
}
bool charging() {
#if defined(HW_HAS_BATTERY_MONITOR)
    return g_charging;
#else
    return false;
#endif
}
}  // namespace BatteryMonitor
