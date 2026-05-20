#include <Arduino.h>
#include <Arduino_GFX_Library.h>

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    5 /* DE */, 3 /* VSYNC */, 46 /* HSYNC */, 7 /* PCLK */,
    14 /* R0 */, 38 /* R1 */, 18 /* R2 */, 17 /* R3 */, 10 /* R4 */,
    39 /* G0 */, 0 /* G1 */, 45 /* G2 */, 48 /* G3 */, 47 /* G4 */, 21 /* G5 */,
    1 /* B0 */, 2 /* B1 */, 42 /* B2 */, 41 /* B3 */, 40 /* B4 */,
    0, 8, 4, 8, 0, 8, 4, 8, 1, 16000000
);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(800, 480, rgbpanel, 0, true);

void setup() {
    Serial.begin(115200);
    gfx->begin();

// Hintergrund schwarz
gfx->fillScreen(BLACK);

// Großer Titel
gfx->setTextColor(WHITE);
gfx->setTextSize(4);
gfx->setCursor(150, 80);
gfx->println("Waveshare ESP32-S3");

// Trennlinie
gfx->drawFastHLine(50, 160, 700, CYAN);

// Untertitel
gfx->setTextColor(YELLOW);
gfx->setTextSize(3);
gfx->setCursor(250, 190);
gfx->println("4.3\" Touch LCD");

// Trennlinie
gfx->drawFastHLine(50, 280, 700, CYAN);

// Grüner Status
gfx->setTextColor(GREEN);
gfx->setTextSize(2);
gfx->setCursor(320, 310);
gfx->println("Display OK!");

Serial.println("Display Hello World OK!");
}

void loop() {
    delay(1000);
}