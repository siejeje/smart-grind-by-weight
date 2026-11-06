// LVGL allocator for ESP32-S3 with PSRAM.
// The Arduino heap keeps every allocation < 4 KB in internal RAM, so thousands of small
// LVGL objects were eating the internal heap that BLE and FreeRTOS task stacks need
// (boot halted when task creation failed). LVGL data goes to PSRAM instead; the draw
// buffer was already there.
#include "lvgl.h"

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM

#include <esp_heap_caps.h>
#include <stdlib.h>

static constexpr uint32_t kCaps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;

extern "C" {

void lv_mem_init(void) {}
void lv_mem_deinit(void) {}

lv_mem_pool_t lv_mem_add_pool(void* mem, size_t bytes) {
    LV_UNUSED(mem);
    LV_UNUSED(bytes);
    return NULL;
}

void lv_mem_remove_pool(lv_mem_pool_t pool) { LV_UNUSED(pool); }

void* lv_malloc_core(size_t size) {
    void* p = heap_caps_malloc(size, kCaps);
    return p ? p : malloc(size);  // fall back to internal RAM if PSRAM is missing/full
}

void* lv_realloc_core(void* p, size_t new_size) {
    if (!p) return lv_malloc_core(new_size);
    void* r = heap_caps_realloc(p, new_size, kCaps);
    return r ? r : realloc(p, new_size);
}

void lv_free_core(void* p) { heap_caps_free(p); }

void lv_mem_monitor_core(lv_mem_monitor_t* mon_p) { LV_UNUSED(mon_p); }

lv_result_t lv_mem_test_core(void) { return LV_RESULT_OK; }

}  // extern "C"

#endif
