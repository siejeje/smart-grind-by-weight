#pragma once

//==============================================================================
// HARDWARE CONFIGURATION CONSTANTS
//==============================================================================
// This file contains hardware-specific configuration values that define the
// physical characteristics and constraints of the ESP32-S3 coffee grinder system.
// These values are tied to the specific hardware implementation and should only
// be modified when changing hardware components.

//------------------------------------------------------------------------------
// GPIO PIN ASSIGNMENTS
//------------------------------------------------------------------------------
#if defined(BOARD_WAVESHARE_ESP32S3_TOUCH_LCD_2)
// Waveshare ESP32-S3-Touch-LCD-2 (ST7789 SPI LCD 240x320 + CST816D touch)
// Pin numbers taken from Waveshare's official demo package (pin_config / EXAMPLE_PIN_NUM_*)
// https://docs.waveshare.com/ESP32-S3-Touch-LCD-2
#define HW_TOUCH_I2C_SDA_PIN 48                                                // I2C SDA (shared with QMI8658 IMU)
#define HW_TOUCH_I2C_SCL_PIN 47                                                // I2C SCL (shared with QMI8658 IMU)
#define HW_TOUCH_I2C_ADDRESS 0x15                                              // I2C address of CST816D touch controller

#define HW_DISPLAY_CS_PIN 45                                                   // LCD SPI chip select
#define HW_DISPLAY_SCK_PIN 39                                                  // LCD SPI clock
#define HW_DISPLAY_MOSI_PIN 38                                                 // LCD SPI MOSI
#define HW_DISPLAY_MISO_PIN 40                                                 // Shared SPI MISO (SD card)
#define HW_DISPLAY_DC_PIN 42                                                   // LCD data/command
#define HW_DISPLAY_RESET_PIN -1                                                // LCD reset not wired to a GPIO (software reset)
#define HW_DISPLAY_BACKLIGHT_PIN 1                                             // LCD backlight (active high, PWM)
#define HW_DISPLAY_BACKLIGHT_PWM_FREQ_HZ 10000
#define HW_DISPLAY_BACKLIGHT_PWM_BITS 10

#define HW_BATTERY_ADC_PIN 5                                                   // Battery voltage via 200k/100k divider (x3)
#else
// Touch Controller (I2C)
#define HW_TOUCH_I2C_SDA_PIN 47                                                // I2C data pin for capacitive touch controller
#define HW_TOUCH_I2C_SCL_PIN 48                                                // I2C clock pin for capacitive touch controller
#define HW_TOUCH_I2C_ADDRESS 0x38                                              // I2C address of FT3168 touch controller

// Display Controller (QSPI)
#define HW_DISPLAY_CS_PIN 9                                                    // SPI chip select for display controller
#define HW_DISPLAY_SCK_PIN 10                                                  // SPI clock for display controller
#define HW_DISPLAY_D0_PIN 11                                                   // SPI data line 0 (QSPI mode)
#define HW_DISPLAY_D1_PIN 12                                                   // SPI data line 1 (QSPI mode)
#define HW_DISPLAY_D2_PIN 13                                                   // SPI data line 2 (QSPI mode)
#define HW_DISPLAY_D3_PIN 14                                                   // SPI data line 3 (QSPI mode)
#define HW_DISPLAY_RESET_PIN 21                                                // Display reset pin

#endif

// Load Cell ADC Pins
#if defined(BOARD_WAVESHARE_ESP32S3_TOUCH_LCD_2)
// GPIO2/3 are camera data lines on this board; use free header pins instead.
// Change these to match your wiring if needed (avoid strapping pins 0, 3, 45, 46).
#define HW_LOADCELL_DOUT_PIN 16                                                // HX711 DOUT
#define HW_LOADCELL_SCK_PIN 15                                                 // HX711 PD_SCK
#else
#define HW_LOADCELL_DOUT_PIN 3                                                 // HX711 data output pin
#define HW_LOADCELL_SCK_PIN 2                                                  // HX711 serial clock pin
#endif

// Motor Control
#define HW_MOTOR_RELAY_PIN 18                                                  // GPIO pin for grinder motor control relay
// Hard safety limit for a single continuous motor activation, enforced inside the
// Grinder driver by an independent esp_timer (in addition to GRIND_TIMEOUT_SEC).
// Lelit William PL72: the add/stop button runs the motor at most ~19 s per hold,
// so stay below that. 0 disables the hard limit.
#ifndef HW_GRINDER_MAX_CONTINUOUS_ON_MS
#if defined(BOARD_WAVESHARE_ESP32S3_TOUCH_LCD_2)
#define HW_GRINDER_MAX_CONTINUOUS_ON_MS 18000
#else
#define HW_GRINDER_MAX_CONTINUOUS_ON_MS 0
#endif
#endif
#define HW_GRINDER_SETTLING_TIME_MS 500                                        // Startup transient immunity (tune based on mechanical rigidity, 0 to disable)

//------------------------------------------------------------------------------
// LOAD CELL ADC SPECIFICATIONS
//------------------------------------------------------------------------------
// Sample rate configuration
#define HW_LOADCELL_SAMPLE_RATE_SPS 10                                         // Current sample rate setting
#define HW_LOADCELL_SAMPLE_INTERVAL_MS (1000 / HW_LOADCELL_SAMPLE_RATE_SPS)   // Calculated sample interval

// Calibration validation
#define HW_LOADCELL_CAL_MIN_ADC_VALUE 1000                                    // Minimum ADC value to confirm weight placed on scale

//------------------------------------------------------------------------------
// DISPLAY SPECIFICATIONS  
//------------------------------------------------------------------------------
#if defined(BOARD_WAVESHARE_ESP32S3_TOUCH_LCD_2)
#define HW_DISPLAY_WIDTH_PX 240                                                // LCD width in pixels (portrait)
#define HW_DISPLAY_HEIGHT_PX 320                                               // LCD height in pixels (portrait)
#define HW_DISPLAY_ROTATION_DEG 0                                              // Arduino_GFX rotation index (0 = portrait)
#define HW_DISPLAY_IPS true                                                    // ST7789 IPS panel
#define HW_UI_CONTENT_WIDTH_PX 220                                             // Width of full-width UI rows
#define HW_UI_BUTTON_SIZE_PX 76                                                // Grind/pulse round button size
#define HW_UI_BUTTON_OFFSET_PX 46                                              // Horizontal offset of the two bottom buttons
#else
#define HW_DISPLAY_WIDTH_PX 280                                                // LCD width in pixels
#define HW_DISPLAY_HEIGHT_PX 456                                               // LCD height in pixels
#define HW_DISPLAY_OFFSET_X_PX 0                                               // X offset for display positioning
#define HW_DISPLAY_ROTATION_DEG 0                                              // Display rotation angle
#define HW_DISPLAY_IPS_INVERT_X 180                                            // IPS X-axis inversion setting
#define HW_DISPLAY_IPS_INVERT_Y 24                                             // IPS Y-axis inversion setting
#define HW_DISPLAY_COLOR_ORDER 20                                              // Color channel ordering
#define HW_UI_CONTENT_WIDTH_PX 280                                             // Width of full-width UI rows
#define HW_UI_BUTTON_SIZE_PX 100                                               // Grind/pulse round button size
#define HW_UI_BUTTON_OFFSET_PX 60                                              // Horizontal offset of the two bottom buttons
#endif
#define HW_DISPLAY_MINIMAL_BRIGHTNESS_PERCENT 15                               // Minimum brightness percentage (to avoid too dim to see)

//------------------------------------------------------------------------------
// SERIAL COMMUNICATION
//------------------------------------------------------------------------------
#define HW_SERIAL_BAUD_RATE 115200                                             // UART baud rate for debug/logging output
