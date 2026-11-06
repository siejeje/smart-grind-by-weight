#pragma once
// Li-ion battery monitor for boards with a VBAT divider on an ADC pin and a charger
// status line (Waveshare ESP32-S3-Touch-LCD-2: 200k/100k divider on GPIO5, CHG_DET on GPIO18).
#include <stdint.h>

namespace BatteryMonitor {
void init();
// Call periodically (internally rate-limited to one measurement per ~1 s).
void update();
bool available();
float voltage();        // filtered battery voltage [V], 0 if no measurement yet
float raw_voltage();    // last unfiltered measurement [V]
int adc_millivolts();   // last ADC pin voltage [mV] (before divider correction)
int percent();          // 0..100, -1 if unknown
bool charging();
// True when the board runs from USB with no cell connected (battery switch off):
// detected from the charger behaviour (CHG line toggling, VBAT jumping or above 4.25 V).
bool usb_only();
int chg_toggles();      // CHG_DET edges in the last detection window (diagnostics)
float vbat_spread();    // max-min VBAT over the detection window [V] (diagnostics)
}  // namespace BatteryMonitor
