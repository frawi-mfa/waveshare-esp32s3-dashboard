#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <WiFi.h>

// WiFi Credentials
const char* WIFI_SSID     = "Tempel-Bau";
const char* WIFI_PASSWORD = "6502199985512364";

// Display Setup
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    5 /* DE */, 3 /* VSYNC */, 46 /* HSYNC */, 7 /* PCLK */,
    14 /* R0 */, 38 /* R1 */, 18 /* R2 */, 17 /* R3 */, 10 /* R4 */,
    39 /* G0 */, 0 /* G1 */, 45 /* G2 */, 48 /* G3 */, 47 /* G4 */, 21 /* G5 */,
    1 /* B0 */, 2 /* B1 */, 42 /* B2 */, 41 /* B3 */, 40 /* B4 */,
    0, 8, 4, 8, 0, 8, 4, 8, 1, 16000000
);
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(800, 480, rgbpanel, 0, true);

void drawStatusScreen() {
    gfx->fillScreen(BLACK);

    // Titel
    gfx->setTextColor(CYAN);
    gfx->setTextSize(3);
    gfx->setCursor(20, 20);
    gfx->println("WiFi Status");
    gfx->drawFastHLine(0, 60, 800, CYAN);

    // SSID
    gfx->setTextColor(YELLOW);
    gfx->setTextSize(2);
    gfx->setCursor(20, 80);
    gfx->print("SSID:     ");
    gfx->setTextColor(WHITE);
    gfx->println(WiFi.SSID());

    // IP
    gfx->setTextColor(YELLOW);
    gfx->setCursor(20, 115);
    gfx->print("IP:       ");
    gfx->setTextColor(GREEN);
    gfx->println(WiFi.localIP().toString());

    // Gateway
    gfx->setTextColor(YELLOW);
    gfx->setCursor(20, 150);
    gfx->print("Gateway:  ");
    gfx->setTextColor(WHITE);
    gfx->println(WiFi.gatewayIP().toString());

    // DNS
    gfx->setTextColor(YELLOW);
    gfx->setCursor(20, 185);
    gfx->print("DNS:      ");
    gfx->setTextColor(WHITE);
    gfx->println(WiFi.dnsIP().toString());

    // Subnet
    gfx->setTextColor(YELLOW);
    gfx->setCursor(20, 220);
    gfx->print("Subnet:   ");
    gfx->setTextColor(WHITE);
    gfx->println(WiFi.subnetMask().toString());

    // MAC
    gfx->setTextColor(YELLOW);
    gfx->setCursor(20, 255);
    gfx->print("MAC:      ");
    gfx->setTextColor(WHITE);
    gfx->println(WiFi.macAddress());

    // RSSI
    int rssi = WiFi.RSSI();
    uint16_t rssiColor = rssi >= -50 ? GREEN : rssi >= -65 ? YELLOW : rssi >= -75 ? ORANGE : RED;
    gfx->setTextColor(YELLOW);
    gfx->setCursor(20, 290);
    gfx->print("Signal:   ");
    gfx->setTextColor(rssiColor);
    gfx->print(rssi);
    gfx->println(" dBm");

    // Kanal
    gfx->setTextColor(YELLOW);
    gfx->setCursor(20, 325);
    gfx->print("Kanal:    ");
    gfx->setTextColor(WHITE);
    gfx->println(WiFi.channel());

    // Trennlinie
    gfx->drawFastHLine(0, 370, 800, DARKGREY);

    // Status
    gfx->setTextColor(GREEN);
    gfx->setTextSize(2);
    gfx->setCursor(20, 385);
    gfx->println("● Verbunden");

    // Uptime
    gfx->setTextColor(DARKGREY);
    gfx->setTextSize(1);
    gfx->setCursor(20, 420);
    gfx->print("Uptime: ");
    gfx->print(millis() / 1000);
    gfx->println(" Sekunden");
}

void showConnecting() {
    gfx->fillScreen(BLACK);
    gfx->setTextColor(CYAN);
    gfx->setTextSize(3);
    gfx->setCursor(20, 20);
    gfx->println("WiFi Status");
    gfx->drawFastHLine(0, 60, 800, CYAN);
    gfx->setTextColor(YELLOW);
    gfx->setTextSize(2);
    gfx->setCursor(20, 100);
    gfx->print("Verbinde mit: ");
    gfx->setTextColor(WHITE);
    gfx->println(WIFI_SSID);
    gfx->setTextColor(ORANGE);
    gfx->setCursor(20, 140);
    gfx->println("Bitte warten...");
}

void setup() {
    Serial.begin(115200);
    gfx->begin();

    showConnecting();

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
        Serial.print(".");
        // Punkte auf Display
        gfx->setTextColor(WHITE);
        gfx->setTextSize(2);
        gfx->setCursor(20 + attempts * 12, 180);
        gfx->print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nVerbunden!");
        drawStatusScreen();
    } else {
        gfx->fillScreen(BLACK);
        gfx->setTextColor(RED);
        gfx->setTextSize(3);
        gfx->setCursor(20, 200);
        gfx->println("Verbindung fehlgeschlagen!");
        Serial.println("\nVerbindung fehlgeschlagen!");
    }
}

void loop() {
    // Status alle 30s aktualisieren
    delay(30000);
    if (WiFi.status() == WL_CONNECTED) {
        drawStatusScreen();
    }
}