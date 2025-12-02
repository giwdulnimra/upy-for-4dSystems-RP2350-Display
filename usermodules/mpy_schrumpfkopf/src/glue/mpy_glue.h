#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque Pointer (Handles) - Function Pointer to divide c/cpp-worlds
typedef struct LedWrapper LedWrapper;
typedef struct ConsoleWrapper ConsoleWrapper;

// LED funkctions
LedWrapper* bridge_led_new(uint32_t pin);
void bridge_led_delete(LedWrapper* w);
void bridge_led_on(LedWrapper* w);
void bridge_led_off(LedWrapper* w);
bool bridge_led_is_on(LedWrapper* w);
uint32_t bridge_led_get_pin(LedWrapper* w);
void bridge_led_print_status(LedWrapper* w);

// Console functions
ConsoleWrapper* bridge_console_new();
void bridge_console_info(ConsoleWrapper* w, const char* msg);

#ifdef __cplusplus
}
#endif