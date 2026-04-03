#include <Arduino.h>
#include "LovyanGFX_config.h"
#include "WiFiProvisioner.h"
#include "OTAManager.h"
#include "JuliaRenderer.h"
#include "JpegFetcher.h"
#include "TouchDimmer.h"

#define REFRESH_RATE 60000

// ----------------------------------------------------------------
// MaTouch Base Application
// Phase 1:  WiFi provisioning + NTP          ✅
// Phase 1b: NTP time display                 ✅
// Phase 2:  Julia set with touch morphing    ✅
// Phase 3:  JPEG download + display          ✅
// Phase 4:  Touch dimming (auto + gesture)   ✅
// ----------------------------------------------------------------

LGFX           display;
WiFiProvisioner wifi;
OTAManager     ota;
JuliaRenderer  julia;
JpegFetcher    fetcher(&display);   // constructed once, not every loop()
TouchDimmer*   dimmer = nullptr;    // heap — needs display dimensions from init

// --- Boot status display ---
static int statusY = 10;
void showStatus(const String& msg) {
    Serial.println(msg);
    display.setTextSize(1);
    display.setTextColor(TFT_GREEN, TFT_BLACK);
    display.setCursor(8, statusY);
    display.print(msg);
    statusY += 14;
    if (statusY > display.height() - 20) {
        display.fillScreen(TFT_BLACK);
        statusY = 10;
    }
}

void setup() {


    delay(4000);
    Serial.begin(115200);

    display.init();
    display.setBrightness(TouchDimmer::BRIGHTNESS_FULL);
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextSize(2);
    display.setCursor(8, 8);
    display.println("MaTouch Base");
    statusY = 32;
    showStatus("Display OK");

    bool wifiOk = wifi.begin([](const String& msg) {
        showStatus(msg);
    });

    if (wifiOk) {
        ota.begin("matouch");
        showStatus("Ready.");
    } else {
        showStatus("No WiFi - offline mode.");
    }

    delay(800);

    Serial.println("\n\n=== MaTouch Base ===");
    Serial.printf("Free heap:  %u\n", ESP.getFreeHeap());
    Serial.printf("Free PSRAM: %u\n", ESP.getFreePsram());
    Serial.printf("PSRAM size: %u\n", ESP.getPsramSize());

    // Dimmer needs real display dimensions (post-rotation: 480 x 320)
    dimmer = new TouchDimmer(
        display.width(),
        display.height(),
        [](uint8_t b) { display.setBrightness(b); }
    );

    display.fillScreen(TFT_BLACK);
}

void loop() {
    ota.handle();
    wifi.maintain();
    dimmer->update();

    // --- Touch handling ---
    // getTouch() returns display-space coords after LovyanGFX applies
    // the touch offset_rotation transform. No raw coordinate math needed.
        uint16_t tx, ty;
    if (display.getTouch(&tx, &ty)) {
        Serial.printf("[touch] x=%3d  y=%3d  corner=%s\n",
            tx, ty,
            (tx <= TouchDimmer::CORNER_PX && ty >= (display.height() - TouchDimmer::CORNER_PX))
                ? "YES" : "no");

        bool consumed = dimmer->onTouch(tx, ty);
        if (!consumed) {
            // Application touch handlers go here.
        }
    }

    // --- JPEG fetch (skip while dimmed — no point refreshing a dark screen) ---
    static uint32_t lastFetch = REFRESH_RATE;
    const char* TEST_URL = "http://10.0.0.10/103_0051.JPG";

    if (!dimmer->isDimmed() && (millis() - lastFetch > REFRESH_RATE)) {
        lastFetch = millis();
        fetcher.fetchAndDraw(TEST_URL);
    }
}