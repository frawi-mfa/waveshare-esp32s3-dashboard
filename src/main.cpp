#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include <time.h>
#include <ESP32Ping.h>
#include "config.h"

// Display Setup
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    5, 3, 46, 7,
    14, 38, 18, 17, 10,
    39, 0, 45, 48, 47, 21,
    1, 2, 42, 41, 40,
    0, 8, 4, 8, 0, 8, 4, 8, 1, 16000000
);
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(800, 480, rgbpanel, 0, true);

// NTP
const char* ntpServer        = "pool.ntp.org";
const long  gmtOffset_sec    = 3600;
const int   daylightOffset_sec = 3600;

// Geräte für Ping
struct Device {
    const char* ip;
    const char* name;
};

Device devices[] = {
    {"192.168.1.1",   "Fritz-Box"},
    {"192.168.1.50",  "Tempel2 File Server"},
    {"192.168.1.33",  "Fritz Repeater 1"},
    {"192.168.1.131", "Fritz Repeater 2"},
    {"192.168.1.40",  "Agfeo Telefonanlage"}
};
const int deviceCount = 5;

String lastTime = "";
String lastDate = "";
int currentPage = 0;
unsigned long lastPageSwitch = 0;
const unsigned long PAGE_DURATION = 10000; // 20 Sekunden

// ─── Seite 1: Dashboard ───────────────────────────────────────────────────────
void drawDashboardStatic() {
    gfx->fillScreen(BLACK);

    gfx->fillRect(0, 0, 800, 55, 0x1082);
    gfx->setTextColor(CYAN);
    gfx->setTextSize(3);
    gfx->setCursor(20, 12);
    gfx->print("ESP32-S3 Dashboard");

    // Seitenindikator
    gfx->setTextColor(DARKGREY);
    gfx->setTextSize(2);
    gfx->setCursor(680, 18);
    gfx->print("1 / 2");

    gfx->drawFastHLine(0, 55, 800, CYAN);

    gfx->setTextColor(YELLOW);
    gfx->setTextSize(2);
    gfx->setCursor(20, 80);  gfx->print("Netzwerk:");
    gfx->setCursor(20, 115); gfx->print("IP:");
    gfx->setCursor(20, 150); gfx->print("Signal:");
    gfx->setCursor(20, 185); gfx->print("MAC:");

    gfx->drawFastHLine(0, 225, 800, DARKGREY);
    gfx->setCursor(20, 245); gfx->print("Datum:");
    gfx->setCursor(20, 305); gfx->print("Uhrzeit:");

    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(200, 80);
    gfx->print(WiFi.SSID());

    gfx->setTextColor(GREEN);
    gfx->setCursor(200, 115);
    gfx->print(WiFi.localIP().toString());

    int rssi = WiFi.RSSI();
    uint16_t rssiColor = rssi >= -50 ? GREEN : rssi >= -65 ? YELLOW : rssi >= -75 ? ORANGE : RED;
    gfx->setTextColor(rssiColor);
    gfx->setCursor(200, 150);
    gfx->print(rssi);
    gfx->print(" dBm");

    gfx->setTextColor(WHITE);
    gfx->setCursor(200, 185);
    gfx->print(WiFi.macAddress());

    gfx->drawFastHLine(0, 460, 800, DARKGREY);
    gfx->setTextColor(DARKGREY);
    gfx->setTextSize(1);
    gfx->setCursor(20, 468);
    gfx->print("Tempel-Bau Nord GmbH  |  ESP32-S3 Network Dashboard v1.1");

    lastTime = "";
    lastDate = "";
}

void updateClock() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    char timeStr[10];
    char dateStr[20];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    strftime(dateStr, sizeof(dateStr), "%d.%m.%Y", &timeinfo);

    String newTime = String(timeStr);
    String newDate = String(dateStr);

    if (newDate != lastDate) {
        gfx->fillRect(200, 240, 400, 35, BLACK);
        gfx->setTextColor(WHITE);
        gfx->setTextSize(3);
        gfx->setCursor(200, 245);
        gfx->print(newDate);
        lastDate = newDate;
    }

    if (newTime != lastTime) {
        gfx->fillRect(200, 295, 500, 65, BLACK);
        gfx->setTextColor(GREEN);
        gfx->setTextSize(5);
        gfx->setCursor(200, 305);
        gfx->print(newTime);
        lastTime = newTime;
    }
}

// ─── Seite 2: Ping Monitor ───────────────────────────────────────────────────
void drawPingPage() {
    gfx->fillScreen(BLACK);

    gfx->fillRect(0, 0, 800, 55, 0x1082);
    gfx->setTextColor(CYAN);
    gfx->setTextSize(3);
    gfx->setCursor(20, 12);
    gfx->print("Netzwerk Monitor");

    gfx->setTextColor(DARKGREY);
    gfx->setTextSize(2);
    gfx->setCursor(680, 18);
    gfx->print("2 / 2");

    gfx->drawFastHLine(0, 55, 800, CYAN);

    // Scanning Meldung
    gfx->setTextColor(YELLOW);
    gfx->setTextSize(2);
    gfx->setCursor(20, 68);
    gfx->print("Ping läuft...");

    for (int i = 0; i < deviceCount; i++) {
        int y = 100 + i * 70;

        // Zebra
        if (i % 2 == 0)
            gfx->fillRect(0, y - 5, 800, 68, 0x1082);

        // Ping ausführen
        bool ok = Ping.ping(devices[i].ip, 1);

        // Status Kreis
        gfx->fillCircle(30, y + 25, 18, ok ? GREEN : RED);
        gfx->setTextColor(ok ? GREEN : RED);
        gfx->setTextSize(2);
        gfx->setCursor(20, y + 18);
        gfx->setTextColor(BLACK);
        gfx->print(ok ? "OK" : "!!");

        // Name
        gfx->setTextColor(WHITE);
        gfx->setTextSize(2);
        gfx->setCursor(65, y + 8);
        gfx->print(devices[i].name);

        // IP
        gfx->setTextColor(DARKGREY);
        gfx->setTextSize(2);
        gfx->setCursor(65, y + 32);
        gfx->print(devices[i].ip);

        // Status Text
        gfx->setTextColor(ok ? GREEN : RED);
        gfx->setTextSize(2);
        gfx->setCursor(600, y + 18);
        gfx->print(ok ? ">> ONLINE" : "!! OFFLINE");
    }

    // Scanning Meldung löschen
    gfx->fillRect(0, 60, 300, 30, BLACK);

    gfx->drawFastHLine(0, 460, 800, DARKGREY);
    gfx->setTextColor(DARKGREY);
    gfx->setTextSize(1);
    gfx->setCursor(20, 468);
    gfx->print("Tempel-Bau Nord GmbH  |  ESP32-S3 Network Dashboard v1.1");
}

// ─── Setup ───────────────────────────────────────────────────────────────────
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
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(1000);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        attempts++;
        gfx->setTextColor(WHITE);
        gfx->setTextSize(2);
        gfx->setCursor(20 + (attempts % 50) * 12, 180);
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

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeinfo;
    int ntpAttempts = 0;
    while (!getLocalTime(&timeinfo) && ntpAttempts < 10) {
        delay(500);
        ntpAttempts++;
    }

    drawDashboardStatic();
    lastPageSwitch = millis();
}

// ─── Loop ─────────────────────────────────────────────────────────────────────
void loop() {
    unsigned long now = millis();

    if (currentPage == 0) {
        updateClock();

        if (now - lastPageSwitch >= PAGE_DURATION) {
            currentPage = 1;
            lastPageSwitch = now;
            drawPingPage();
        }
    } else {
        if (now - lastPageSwitch >= PAGE_DURATION) {
            currentPage = 0;
            lastPageSwitch = now;
            drawDashboardStatic();
        }
    }

    delay(1000);
}