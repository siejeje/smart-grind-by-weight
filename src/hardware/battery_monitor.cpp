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
    bool chg = digitalRead(HW_CHARGE_DETECT_PIN) == HIGH;

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
bool charging() {
#if defined(HW_HAS_BATTERY_MONITOR)
    return g_charging;
#else
    return false;
#endif
}
}  // namespace BatteryMonitor
