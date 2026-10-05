// T-Display-S3 status screen, kept out of the DNS critical path.
// Parallel 8-bit ST7789 via Arduino_GFX (see Arduino_GFX wiki Dev-Device-Declaration).
// Power pin 15 must go HIGH, backlight is GPIO 38. Buttons: BOOT/GPIO 0 and GPIO 14.
#ifdef DISPLAY_ST7789

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include "display.h"

#define GFX_PWD 15
#define GFX_BL 38

static Arduino_DataBus* gfxBus = nullptr;
static Arduino_GFX* gfx = nullptr;
static int displayPage = 0;
static uint32_t lastDrawMs = 0;
static uint32_t lastBtnMs = 0;

void displayInit() {
  pinMode(GFX_PWD, OUTPUT);
  digitalWrite(GFX_PWD, HIGH);
  pinMode(0, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  gfxBus = new Arduino_ESP32PAR8Q(
      7 /* DC */, 6 /* CS */, 8 /* WR */, 9 /* RD */,
      39 /* D0 */, 40 /* D1 */, 41 /* D2 */, 42 /* D3 */,
      45 /* D4 */, 46 /* D5 */, 47 /* D6 */, 48 /* D7 */);
  gfx = new Arduino_ST7789(gfxBus, 5 /* RST */, 0 /* rotation */, true /* IPS */,
                           170 /* w */, 320 /* h */, 35, 0, 35, 0);
  if (!gfx->begin()) return;
  gfx->fillScreen(BLACK);
  gfx->setCursor(8, 12);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  gfx->println("ESP32 AdBlock");
  gfx->setTextSize(1);
  gfx->setTextColor(GREEN);
  gfx->println("booting...");
}

static void drawMain(bool blockingOn, uint32_t blocked, uint32_t allowed,
                     int clients, const char* ip) {
  gfx->fillScreen(BLACK);
  gfx->setCursor(8, 10);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  gfx->println("ESP32 AdBlock");
  gfx->setTextSize(1);
  gfx->setTextColor(blockingOn ? GREEN : ORANGE);
  gfx->printf("Blocking: %s\n", blockingOn ? "ON" : "PAUSED");
  gfx->setTextColor(CYAN);
  gfx->printf("IP %s\n", ip ? ip : "-");
  gfx->setTextColor(WHITE);
  gfx->printf("Blocked %u\n", blocked);
  gfx->printf("Allowed %u\n", allowed);
  gfx->printf("Clients %d\n", clients);
  uint32_t up = millis() / 1000;
  gfx->printf("Uptime %lud %luh %lum\n", up / 86400, (up % 86400) / 3600, (up % 3600) / 60);
  gfx->setTextColor(DARKGREY);
  gfx->println("BTN2: next page");
}

static void drawSystem(uint32_t domains) {
  gfx->fillScreen(BLACK);
  gfx->setCursor(8, 10);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  gfx->println("System");
  gfx->setTextSize(1);
  gfx->setTextColor(YELLOW);
  gfx->printf("Domains %u\n", domains);
  gfx->printf("RSSI %d dBm\n", WiFi.RSSI());
  gfx->printf("Heap %u KB\n", (unsigned)(ESP.getFreeHeap() / 1024));
#ifdef BOARD_HAS_PSRAM
  gfx->printf("PSRAM %u KB\n", (unsigned)(ESP.getFreePsram() / 1024));
#else
  gfx->println("PSRAM n/a");
#endif
  gfx->setTextColor(DARKGREY);
  gfx->println("BTN1: prev page");
}

void displayTick(bool blockingOn, uint32_t blocked, uint32_t allowed,
                 uint32_t domains, int clients, const char* ip) {
  if (!gfx) return;
  uint32_t now = millis();
  // Buttons: non-blocking debounce, switch pages.
  if (now - lastBtnMs > 200) {
    if (digitalRead(14) == LOW) { displayPage = (displayPage + 1) % 2; lastBtnMs = now; }
    else if (digitalRead(0) == LOW) { displayPage = (displayPage + 1) % 2; lastBtnMs = now; }
  }
  if (now - lastDrawMs < 2000) return;
  lastDrawMs = now;
  if (displayPage == 0) drawMain(blockingOn, blocked, allowed, clients, ip);
  else drawSystem(domains);
}

#endif  // DISPLAY_ST7789
