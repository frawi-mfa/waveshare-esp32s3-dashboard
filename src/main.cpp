#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include <time.h>
#include <ESP32Ping.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
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
Device devices[] = PING_DEVICES;
const int deviceCount = PING_DEVICE_COUNT;

// BME280 Innen
Adafruit_BME280 bme;
float inTemp = 0.0;
float inHum  = 0.0;

// BLE UUIDs
#define SERVICE_UUID  "12345678-1234-1234-1234-123456789abc"
#define TEMP_UUID     "12345678-1234-1234-1234-123456789ab1"
#define HUM_UUID      "12345678-1234-1234-1234-123456789ab2"
#define PRES_UUID     "12345678-1234-1234-1234-123456789ab3"

// BLE Variablen
static BLERemoteCharacteristic* tempChar;
static BLERemoteCharacteristic* humChar;
static BLERemoteCharacteristic* presChar;
static BLEAdvertisedDevice* myDevice;
bool doConnect = false;
bool connected = false;
bool doScan = false;

float bleTemp = 0.0;
float bleHum  = 0.0;
float blePres = 0.0;
String bleStatus = "Suche...";

// Seiten
String lastTime = "";
String lastDate = "";
int currentPage = 0;
unsigned long lastPageSwitch = 0;
const unsigned long PAGE_DURATION = 10000;

// ─── BLE Callbacks ────────────────────────────────────────────────────────────
class ClientCallbacks : public BLEClientCallbacks {
    void onConnect(BLEClient* client) { connected = true; }
    void onDisconnect(BLEClient* client) {
        connected = false;
        bleStatus = "Getrennt";
        doScan = true;
    }
};

class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.getName() == "ESP32-C3 Wetterstation") {
            BLEDevice::getScan()->stop();
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            bleStatus = "Gefunden!";
        }
    }
};

bool connectToServer() {
    BLEClient* client = BLEDevice::createClient();
    client->setClientCallbacks(new ClientCallbacks());
    client->connect(myDevice);

    BLERemoteService* service = client->getService(SERVICE_UUID);
    if (service == nullptr) return false;

    tempChar = service->getCharacteristic(TEMP_UUID);
    humChar  = service->getCharacteristic(HUM_UUID);
    presChar = service->getCharacteristic(PRES_UUID);

    if (tempChar == nullptr || humChar == nullptr || presChar == nullptr) return false;

    bleStatus = "Verbunden";
    return true;
}

void startBLEScan() {
    BLEScan* scan = BLEDevice::getScan();
    scan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
    scan->setInterval(1349);
    scan->setWindow(449);
    scan->setActiveScan(true);
    scan->start(10, false);
}

// ─── Seite 1: Dashboard ───────────────────────────────────────────────────────
void drawDashboardStatic() {
    gfx->fillScreen(BLACK);
    gfx->fillRect(0, 0, 800, 55, 0x1082);
    gfx->setTextColor(CYAN);
    gfx->setTextSize(3);
    gfx->setCursor(20, 12);
    gfx->print("ESP32-S3 Dashboard");
    gfx->setTextColor(DARKGREY);
    gfx->setTextSize(2);
    gfx->setCursor(650, 18);
    gfx->print("1 / 3");
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
    gfx->print("Tempel-Bau Nord GmbH  |  ESP32-S3 Network Dashboard v1.3");

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

// ─── Seite 2: Ping Monitor ────────────────────────────────────────────────────
void drawPingPage() {
    gfx->fillScreen(BLACK);
    gfx->fillRect(0, 0, 800, 55, 0x1082);
    gfx->setTextColor(CYAN);
    gfx->setTextSize(3);
    gfx->setCursor(20, 12);
    gfx->print("Netzwerk Monitor");
    gfx->setTextColor(DARKGREY);
    gfx->setTextSize(2);
    gfx->setCursor(650, 18);
    gfx->print("2 / 3");
    gfx->drawFastHLine(0, 55, 800, CYAN);

    gfx->setTextColor(YELLOW);
    gfx->setTextSize(2);
    gfx->setCursor(20, 68);
    gfx->print("Ping läuft...");

    for (int i = 0; i < deviceCount; i++) {
        int y = 100 + i * 70;
        if (i % 2 == 0)
            gfx->fillRect(0, y - 5, 800, 68, 0x1082);

        bool ok = Ping.ping(devices[i].ip, 1);
 
        gfx->fillCircle(30, y + 25, 18, ok ? GREEN : RED);
        gfx->setTextColor(BLACK);
        gfx->setTextSize(2);
        gfx->setCursor(14, y + 20);
        gfx->print(ok ? "OK" : "!!");

        gfx->setTextColor(WHITE);
        gfx->setCursor(65, y + 8);
        gfx->print(devices[i].name);

        gfx->setTextColor(DARKGREY);
        gfx->setCursor(65, y + 32);
        gfx->print(devices[i].ip);

        gfx->setTextColor(ok ? GREEN : RED);
        gfx->setCursor(600, y + 18);
        gfx->print(ok ? ">> ONLINE" : "!! OFFLINE");
    }

    gfx->fillRect(0, 60, 300, 30, BLACK);
    gfx->drawFastHLine(0, 460, 800, DARKGREY);
    gfx->setTextColor(DARKGREY);
    gfx->setTextSize(1);
    gfx->setCursor(20, 468);
    gfx->print("Tempel-Bau Nord GmbH  |  ESP32-S3 Network Dashboard v1.3");
}

// ─── Seite 3: Wetterstation Innen/Außen ───────────────────────────────────────
void drawWeatherPage() {
    // Innen BME280 lesen
    inTemp = bme.readTemperature();
    inHum  = bme.readHumidity();

    gfx->fillScreen(BLACK);
    gfx->fillRect(0, 0, 800, 55, 0x1082);
    gfx->setTextColor(CYAN);
    gfx->setTextSize(3);
    gfx->setCursor(20, 12);
    gfx->print("Wetterstation");
    gfx->setTextColor(DARKGREY);
    gfx->setTextSize(2);
    gfx->setCursor(650, 18);
    gfx->print("3 / 3");
    gfx->drawFastHLine(0, 55, 800, CYAN);

    // Spalten Header
    gfx->setTextColor(YELLOW);
    gfx->setTextSize(2);
    gfx->setCursor(300, 65);
    gfx->print("INNEN");
    gfx->setCursor(580, 65);
    gfx->print("AUSSEN");

    // BLE Status klein
    gfx->setTextColor(connected ? GREEN : ORANGE);
    gfx->setTextSize(1);
    gfx->setCursor(580, 58);
    gfx->print(connected ? "(BLE OK)" : "(BLE: suche)");

    gfx->drawFastHLine(0, 90, 800, DARKGREY);

    // Temperatur
    gfx->setTextColor(YELLOW);
    gfx->setTextSize(2);
    gfx->setCursor(20, 110);
    gfx->print("Temperatur:");
    gfx->setTextColor(WHITE);
    gfx->setTextSize(3);
    gfx->setCursor(250, 105);
    gfx->printf("%.1f C", inTemp);
    gfx->setTextColor(connected ? WHITE : DARKGREY);
    gfx->setCursor(530, 105);
    gfx->printf("%.1f C", bleTemp);

    gfx->drawFastHLine(0, 150, 800, 0x1082);

    // Luftfeuchtigkeit
    gfx->setTextColor(YELLOW);
    gfx->setTextSize(2);
    gfx->setCursor(20, 175);
    gfx->print("Luftfeuchte:");
    gfx->setTextColor(CYAN);
    gfx->setTextSize(3);
    gfx->setCursor(250, 170);
    gfx->printf("%.1f %%", inHum);
    gfx->setTextColor(connected ? CYAN : DARKGREY);
    gfx->setCursor(530, 170);
    gfx->printf("%.1f %%", bleHum);

    gfx->drawFastHLine(0, 215, 800, 0x1082);

    // Luftdruck nur Außen
    gfx->setTextColor(YELLOW);
    gfx->setTextSize(2);
    gfx->setCursor(20, 240);
    gfx->print("Luftdruck:");
    gfx->setTextColor(connected ? GREEN : DARKGREY);
    gfx->setTextSize(3);
    gfx->setCursor(530, 235);
    gfx->printf("%.1f hPa", blePres);

    gfx->drawFastHLine(0, 280, 800, 0x1082);

    // Differenz Innen/Außen Temperatur
    if (connected) {
        float diffTemp = inTemp - bleTemp;
        float diffHum  = inHum - bleHum;
        gfx->setTextColor(DARKGREY);
        gfx->setTextSize(2);
        gfx->setCursor(20, 305);
        gfx->print("Differenz:");
        gfx->setTextColor(diffTemp > 0 ? ORANGE : CYAN);
        gfx->setCursor(250, 300);
        gfx->printf("%+.1f C", diffTemp);
        gfx->setTextColor(diffHum > 0 ? ORANGE : CYAN);
        gfx->setCursor(530, 300);
        gfx->printf("%+.1f %%", diffHum);
    }

    gfx->drawFastHLine(0, 460, 800, DARKGREY);
    gfx->setTextColor(DARKGREY);
    gfx->setTextSize(1);
    gfx->setCursor(20, 468);
    gfx->print("Tempel-Bau Nord GmbH  |  ESP32-S3 Network Dashboard v1.3");
}

// ─── Setup ────────────────────────────────────────────────────────────────────
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

    // BME280 Innen initialisieren
    Wire.begin(8, 9);
    if (!bme.begin(0x76, &Wire)) {
        Serial.println("BME280 nicht gefunden!");
    } else {
        Serial.println("BME280 Innen OK!");
    }

    // BLE initialisieren
    BLEDevice::init("");
    startBLEScan();

    drawDashboardStatic();
    lastPageSwitch = millis();
}

// ─── Loop ─────────────────────────────────────────────────────────────────────
void loop() {
    unsigned long now = millis();

    // Alle 30 Sekunden neu scannen wenn nicht verbunden
    static unsigned long lastScan = 0;
    if (!connected && !doConnect && (now - lastScan > 30000)) {
        lastScan = now;
        doScan = true;
    }

    // BLE verbinden
    if (doConnect) {
        connectToServer();
        doConnect = false;
    }

    // BLE Daten lesen
    if (connected && tempChar && humChar && presChar) {
        bleTemp = atof(tempChar->readValue().c_str());
        bleHum  = atof(humChar->readValue().c_str());
        blePres = atof(presChar->readValue().c_str());
    }

    // Neu scannen wenn getrennt
    if (doScan) {
        startBLEScan();
        doScan = false;
    }

    // Seiten wechseln
    if (currentPage == 0) {
        updateClock();
        if (now - lastPageSwitch >= PAGE_DURATION) {
            currentPage = 1;
            lastPageSwitch = now;
            drawPingPage();
       }
    } else if (currentPage == 1) {
        // Ping alle 5 Sekunden neu zeichnen
        static unsigned long lastPing = 0;
        if (now - lastPing > 5000) {
            lastPing = now;
            drawPingPage();
        }
        if (now - lastPageSwitch >= PAGE_DURATION) {
            currentPage = 2;
            lastPageSwitch = now;
            drawWeatherPage();
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