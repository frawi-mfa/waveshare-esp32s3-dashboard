#include <Arduino.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>
#include <ESP_IOExpander_Library.h>

// Display Setup
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    5, 3, 46, 7,
    14, 38, 18, 17, 10,
    39, 0, 45, 48, 47, 21,
    1, 2, 42, 41, 40,
    0, 8, 4, 8, 0, 8, 4, 8, 1, 16000000
);
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(800, 480, rgbpanel, 0, true);

// CH422G IO Expander
ESP_IOExpander_CH422G *expander = nullptr;
#define TOUCH_SDA  8
#define TOUCH_SCL  9
#define TP_RST     1
#define LCD_BL     2
#define GT911_ADDR 0x5D

// GT911 direkt lesen
bool gt911_read(int16_t *x, int16_t *y) {
    // Status lesen
    Wire1.beginTransmission(GT911_ADDR);
    Wire1.write(0x81);
    Wire1.write(0x4E);
    Wire1.endTransmission();
    Wire1.requestFrom(GT911_ADDR, 1);
    if (!Wire1.available()) return false;
    uint8_t status = Wire1.read();
    
    // Status löschen
    Wire1.beginTransmission(GT911_ADDR);
    Wire1.write(0x81);
    Wire1.write(0x4E);
    Wire1.write(0x00);
    Wire1.endTransmission();

    uint8_t count = status & 0x0F;
    if (!(status & 0x80) || count == 0) return false;

    // Touch Punkt lesen
Wire1.beginTransmission(GT911_ADDR);
Wire1.write(0x81);
Wire1.write(0x50);
Wire1.endTransmission();
Wire1.requestFrom(GT911_ADDR, 6);
if (Wire1.available() < 6) return false;
uint8_t b[6];
for(int i=0; i<6; i++) b[i] = Wire1.read();
*x = b[0] | (b[1] << 8);
*y = b[2] | (b[3] << 8);

return true;
}


void setup() {
    Serial.begin(115200);
    gfx->begin();
    gfx->fillScreen(BLACK);

    // IO Expander initialisieren
    expander = new ESP_IOExpander_CH422G(TOUCH_SCL, TOUCH_SDA, 0x24);
    expander->init();
    expander->begin();

    // Touch Reset länger
    expander->pinMode(TP_RST, OUTPUT);
    expander->digitalWrite(TP_RST, LOW);
    delay(100);  // ← 10 auf 100 erhöhen
    expander->digitalWrite(TP_RST, HIGH);
    delay(500);  // ← 300 auf 500 erhöhen

    // Backlight an
    expander->pinMode(LCD_BL, OUTPUT);
    expander->digitalWrite(LCD_BL, HIGH);

    // Wire für GT911 direkt
    Wire1.begin(TOUCH_SDA, TOUCH_SCL);

    gfx->setTextColor(GREEN);
    gfx->setTextSize(3);
    gfx->setCursor(20, 20);
    gfx->print("Touch Test - tippe!");

    Serial.println("Setup fertig!");
}

void loop() {
    int16_t x, y;
    if (gt911_read(&x, &y)) {
        Serial.printf("Touch: X=%d Y=%d\n", x, y);
        gfx->fillCircle(x, y, 15, YELLOW);
        gfx->fillRect(0, 440, 800, 40, BLACK);
        gfx->setTextColor(CYAN);
        gfx->setTextSize(2);
        gfx->setCursor(20, 450);
        gfx->printf("X=%d  Y=%d", x, y);
    }
    delay(50);
}