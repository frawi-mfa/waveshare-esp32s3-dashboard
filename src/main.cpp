#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"

// Display Setup
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    5 /* DE */, 3 /* VSYNC */, 46 /* HSYNC */, 7 /* PCLK */,
    14 /* R0 */, 38 /* R1 */, 18 /* R2 */, 17 /* R3 */, 10 /* R4 */,
    39 /* G0 */, 0 /* G1 */, 45 /* G2 */, 48 /* G3 */, 47 /* G4 */, 21 /* G5 */,
    1 /* B0 */, 2 /* B1 */, 42 /* B2 */, 41 /* B3 */, 40 /* B4 */,
    0, 8, 4, 8, 0, 8, 4, 8, 1, 16000000
);
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(800, 480, rgbpanel, 0, true);

// NTP Server
const char* ntpServer     = "pool.ntp.org";
const long  gmtOffset_sec = 3600;      // UTC+1 (Deutschland)
const int   daylightOffset_sec = 3600; // Sommerzeit +1h

String lastTime = "";
String lastDate = "";

void drawStaticUI() {
    gfx->fillScreen(BLACK);

    // Titelleiste
    gfx->fillRect(0, 0, 800, 55, 0x1082);
    gfx->setTextColor(CYAN);
    gfx->setTextSize(3);
    gfx->setCursor(20, 12);
    gfx->print("ESP32-S3 Dashboard");

    // Trennlinie
    gfx->drawFastHLine(0, 55, 800, CYAN);

    // Labels
    gfx->setTextColor(YELLOW);
    gfx->setTextSize(2);

    gfx->setCursor(20, 100);
    gfx->print("Netzwerk:");
    gfx->setCursor(20, 140);
    gfx->print("IP:");
    gfx->setCursor(20, 180);
    gfx->print("Signal:");
    gfx->setCursor(20, 220);
    gfx->print("MAC:");

    gfx->drawFastHLine(0, 260, 800, DARKGREY);

    gfx->setTextColor(YELLOW);
    gfx->setCursor(20, 280);
    gfx->print("Datum:");
    gfx->setCursor(20, 340);
    gfx->print("Uhrzeit:");

    // Footer
    gfx->drawFastHLine(0, 460, 800, DARKGREY);
    gfx->setTextColor(DARKGREY);
    gfx->setTextSize(1);
    gfx->setCursor(20, 468);
    gfx->print("Tempel-Bau Nord GmbH  |  ESP32-S3 Network Dashboard v1.0");

    // Netzwerk-Infos (statisch)
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);

    gfx->setCursor(200, 100);
    gfx->print(WiFi.SSID());

    gfx->setCursor(200, 140);
    gfx->setTextColor(GREEN);
    gfx->print(WiFi.localIP().toString());

    int rssi = WiFi.RSSI();
    uint16_t rssiColor = rssi >= -50 ? GREEN : rssi >= -65 ? YELLOW : rssi >= -75 ? ORANGE : RED;
    gfx->setTextColor(rssiColor);
    gfx->setCursor(200, 180);
    gfx->print(rssi);
    gfx->print(" dBm");

    gfx->setTextColor(WHITE);
    gfx->setCursor(200, 220);
    gfx->print(WiFi.macAddress());
}

void updateClock() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("NTP Fehler");
        return;
    }

    char timeStr[10];
    char dateStr[20];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    strftime(dateStr, sizeof(dateStr), "%d.%m.%Y", &timeinfo);

    String newTime = String(timeStr);
    String newDate = String(dateStr);

    // Datum nur bei Änderung neu zeichnen
    if (newDate != lastDate) {
        gfx->fillRect(200, 275, 400, 35, BLACK);
        gfx->setTextColor(WHITE);
        gfx->setTextSize(3);
        gfx->setCursor(200, 280);
        gfx->print(newDate);
        lastDate = newDate;
    }

    // Uhrzeit nur bei Änderung neu zeichnen
    if (newTime != lastTime) {
        gfx->fillRect(200, 330, 500, 60, BLACK);
        gfx->setTextColor(GREEN);
        gfx->setTextSize(5);
        gfx->setCursor(200, 340);
        gfx->print(newTime);
        lastTime = newTime;
    }
}

void showConnecting() {
    gfx->fillScreen(BLACK);
    gfx->setTextColor(CYAN);
    gfx->setTextSize(3);
    gfx->setCursor(20, 20);
    gfx->println("ESP32-S3 Dashboard");
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
    WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
    WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(1000);

    int attempts = 0;
     while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(1000);  // 500 → 1000ms
        attempts++;
        Serial.print("Versuch ");
        Serial.print(attempts);
        Serial.print(" Status: ");
        Serial.println(WiFi.status());
        gfx->setTextColor(WHITE);
        gfx->setTextSize(2);
        gfx->setCursor(20 + attempts * 12, 180);
        gfx->print(".");
    }

    if (WiFi.status() != WL_CONNECTED) {
        gfx->fillScreen(BLACK);
        gfx->setTextColor(RED);
        gfx->setTextSize(3);
        gfx->setCursor(20, 200);
        gfx->println("Verbindung fehlgeschlagen!");
        return;
    }

    // NTP synchronisieren
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // Warten bis Zeit synchronisiert
    gfx->setTextColor(YELLOW);
    gfx->setTextSize(2);
    gfx->setCursor(20, 220);
    gfx->println("Synchronisiere Zeit...");

    struct tm timeinfo;
    int ntpAttempts = 0;
    while (!getLocalTime(&timeinfo) && ntpAttempts < 40) {
        delay(500);
        ntpAttempts++;
    }

    drawStaticUI();
}

void loop() {
    // WiFi Verbindung prüfen und ggf. neu verbinden
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi verloren - reconnecting...");
        gfx->fillRect(0, 460, 800, 20, BLACK);
        gfx->setTextColor(RED);
        gfx->setTextSize(1);
        gfx->setCursor(20, 468);
        gfx->print("WiFi getrennt - verbinde neu...");
        
        WiFi.disconnect();
        delay(1000);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(1000);
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Reconnect OK!");
            configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.google.com");
            delay(2000);
            drawStaticUI();
        }
        return;
    }

    updateClock();
    delay(1000);
}