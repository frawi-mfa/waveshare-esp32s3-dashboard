#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <WiFi.h>

// Display Setup
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    5 /* DE */, 3 /* VSYNC */, 46 /* HSYNC */, 7 /* PCLK */,
    14 /* R0 */, 38 /* R1 */, 18 /* R2 */, 17 /* R3 */, 10 /* R4 */,
    39 /* G0 */, 0 /* G1 */, 45 /* G2 */, 48 /* G3 */, 47 /* G4 */, 21 /* G5 */,
    1 /* B0 */, 2 /* B1 */, 42 /* B2 */, 41 /* B3 */, 40 /* B4 */,
    0, 8, 4, 8, 0, 8, 4, 8, 1, 16000000
);
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(800, 480, rgbpanel, 0, true);

// Signalstärke → Balken (1-4)
int rssiToBars(int rssi) {
    if (rssi >= -50) return 4;
    if (rssi >= -65) return 3;
    if (rssi >= -75) return 2;
    return 1;
}

// Signalstärke → Farbe
uint16_t rssiToColor(int rssi) {
    if (rssi >= -50) return GREEN;
    if (rssi >= -65) return YELLOW;
    if (rssi >= -75) return ORANGE;
    return RED;
}

void drawSignalBars(int x, int y, int bars, uint16_t color) {
    for (int i = 0; i < 4; i++) {
        int barH = (i + 1) * 5;
        int barX = x + i * 8;
        int barY = y + 20 - barH;
        if (i < bars)
            gfx->fillRect(barX, barY, 6, barH, color);
        else
            gfx->fillRect(barX, barY, 6, barH, DARKGREY);
    }
}

void scanWiFi() {
    gfx->fillScreen(BLACK);

    // Titel
    gfx->setTextColor(CYAN);
    gfx->setTextSize(3);
    gfx->setCursor(20, 20);
    gfx->println("WiFi Scanner");
    gfx->drawFastHLine(0, 60, 800, CYAN);

    // Scannen
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(20, 75);
    gfx->print("Scanning...");

    int n = WiFi.scanNetworks();

    gfx->fillRect(0, 65, 800, 30, BLACK);

    if (n == 0) {
        gfx->setTextColor(RED);
        gfx->setCursor(20, 75);
        gfx->print("Keine Netzwerke gefunden!");
        return;
    }

    // Header
    gfx->setTextColor(YELLOW);
    gfx->setTextSize(2);
    gfx->setCursor(20, 70);
    gfx->print("SSID");
    gfx->setCursor(550, 70);
    gfx->print("RSSI");
    gfx->setCursor(650, 70);
    gfx->print("Signal");
    gfx->setCursor(760, 70);
    gfx->print("🔒");
    gfx->drawFastHLine(0, 95, 800, DARKGREY);

    // Max 8 Netzwerke anzeigen
    int maxNetworks = min(n, 8);
    for (int i = 0; i < maxNetworks; i++) {
        int y = 105 + i * 45;
        int rssi = WiFi.RSSI(i);
        uint16_t color = rssiToColor(rssi);
        int bars = rssiToBars(rssi);

        // Zebra-Hintergrund
        if (i % 2 == 0)
            gfx->fillRect(0, y - 5, 800, 44, 0x1082);

        // SSID
        gfx->setTextColor(WHITE);
        gfx->setTextSize(2);
        gfx->setCursor(20, y + 10);
        String ssid = WiFi.SSID(i);
        if (ssid.length() > 28) ssid = ssid.substring(0, 28) + "..";
        gfx->print(ssid);

        // RSSI Wert
        gfx->setTextColor(color);
        gfx->setCursor(550, y + 10);
        gfx->print(rssi);
        gfx->print(" dBm");

        // Signalbalken
        drawSignalBars(660, y + 5, bars, color);

        // Verschlüsselung
        gfx->setTextColor(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? GREEN : ORANGE);
        gfx->setCursor(760, y + 10);
        gfx->print(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "WPA");
    }

    // Footer
    gfx->drawFastHLine(0, 470, 800, DARKGREY);
    gfx->setTextColor(DARKGREY);
    gfx->setTextSize(1);
    gfx->setCursor(20, 473);
    gfx->print("Gefunden: ");
    gfx->print(n);
    gfx->print(" Netzwerke  |  Scan wiederholt in 10s");

    Serial.print("Gefunden: ");
    Serial.println(n);
}

void setup() {
    Serial.begin(115200);
    gfx->begin();
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    scanWiFi();
}

void loop() {
    delay(10000);
    scanWiFi();
}