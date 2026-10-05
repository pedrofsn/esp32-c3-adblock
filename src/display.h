#pragma once
#include <stdint.h>

// Isolated status UI for LILYGO T-Display-S3 (ST7789 170x320).
// The DNS path never calls into the display; main.cpp polls displayTick()
// from loop() at most every 2 seconds. Buttons switch pages without blocking.
#ifdef DISPLAY_ST7789

void displayInit();
void displayTick(bool blockingOn, uint32_t blocked, uint32_t allowed,
                 uint32_t domains, int clients, const char* ip);

#else

inline void displayInit() {}
inline void displayTick(bool, uint32_t, uint32_t, uint32_t, int, const char*) {}

#endif
