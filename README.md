# Waveshare ESP32-S3 Network Dashboard

Netzwerk- und Wetter-Dashboard auf Basis des Waveshare ESP32-S3-Touch-LCD-4.3 mit 
4.3" Touch Display (800×480). Zeigt Netzwerkstatus, Ping-Monitor, NTP Uhrzeit,
Wetterdaten mit BLE Außensensor und Tages-Statistiken.

## Hardware

| Komponente | Details |
|---|---|
| **Board** | Waveshare ESP32-S3-Touch-LCD-4.3 |
| **Display** | 4.3" IPS RGB, 800×480 Pixel, kapazitiver Touch (GT911) |
| **Chip** | ESP32-S3-WROOM-1, 240MHz, 16MB Flash, 8MB PSRAM |
| **Touch Controller** | GT911 via I2C (Wire1, GPIO8/9) |
| **IO Expander** | CH422G für Touch Reset und Backlight |
| **Innensensor** | BME280 am I2C Header (GPIO8/9, Adresse 0x76) |
| **Außensensor** | ESP32-C3 Mini mit BME280 per BLE |

## Features

### Seite 1 — ESP32-S3 Dashboard
- 📡 SSID und IP-Adresse
- 📶 WiFi Signalstärke (RSSI)
- 🔌 MAC-Adresse
- 📅 Datum und Uhrzeit (NTP, deutsche Zeitzone)

### Seite 2 — Netzwerk Monitor (nur per Touch)
- Ping Monitor für alle konfigurierten Geräte
- Farbige Status Anzeige (ONLINE/OFFLINE)
- Nur per Touch erreichbar — blockiert nicht den Touch Flow

### Seite 3 — Wetterstation
- 🌡️ Temperatur Innen/Außen
- 💧 Luftfeuchtigkeit Innen/Außen
- 🔵 Luftdruck (Außen per BLE)
- 📊 Differenz Innen/Außen
- BLE Status Anzeige

### Seite 4 — Wetter Statistik
- Min/Max Werte seit letztem Start
- Temperatur Innen/Außen
- Luftfeuchtigkeit Innen/Außen
- Luftdruck

## Touch Navigation
- **Tippen** → nächste Seite
- Auto-Wechsel: Seite 1 → Seite 3 alle 10 Sekunden
- Seite 2 (Ping) und Seite 4 (Statistik) nur per Touch

## Software

- **Framework:** Arduino (PlatformIO)
- **Platform:** espressif32@^6.11.0
- **C++ Standard:** C++17 (wegen ESP32_IO_Expander Library)
- **Libraries:**
  - moononournation/GFX Library for Arduino@1.4.9
  - marian-craciunescu/ESP32Ping
  - Adafruit BME280
  - ESP32_IO_Expander (lokal in /lib)
  - esp-lib-utils (lokal in /lib)

## Lokale Libraries (in /lib)
Diese Libraries sind nicht im PlatformIO Registry und müssen 
aus dem Waveshare Demo Paket kopiert werden:
- `ESP32_IO_Expander` — CH422G IO Expander Treiber
- `esp-lib-utils` — Hilfsfunktionen

## Konfiguration
Credentials und Geräteliste in `src/config.h` (nicht im Repo):

```cpp
#define WIFI_SSID     "dein_netzwerk"
#define WIFI_PASSWORD "dein_passwort"

#define PING_DEVICES { \
    {"192.168.1.1", "Fritz-Box"} \
}
#define PING_DEVICE_COUNT 1
```

## Wichtige Hinweise

### Touch (GT911)
- GT911 direkt über Wire1 angesprochen (kein bb_captouch!)
- CH422G IO-Expander Reset muss VOR Wire1.begin() erfolgen
- Wire1 wird von Touch GT911 UND BME280 Innensensor geteilt

### Display RGB Pins
DE=5, VSYNC=3, HSYNC=46, PCLK=7

R: 14,38,18,17,10

G: 39,0,45,48,47,21

B: 1,2,42,41,40

### BLE Außensensor
- Sucht nach BLE Device "ESP32-C3 Wetterstation"
- Automatischer Rescan alle 30 Sekunden
- Repository: [esp32c3-bme280-ble](https://github.com/frawi-mfa/esp32c3-bme280-ble)

## Bekannte Probleme / Lösungen

- **bb_captouch funktioniert nicht** zusammen mit ESP_IOExpander → GT911 direkt ansprechen
- **Wire Konflikt:** Expander nutzt I2C Port 0, Touch+BME280 auf Wire1 (Port 1)
- **PSRAM Warnung** beim Start ist harmlos
- **Ping blockiert Touch** → Ping Seite aus Auto-Wechsel entfernt

## Geplante Erweiterungen

- Asynchrones Pingen für bessere Touch Reaktion
- Weitere BLE Sensor Nodes (Schlafzimmer, Küche)
- Telegram Alarm für Serverraum Temperatur

## Projektkontext

Teil eines größeren ESP32 Sensor-Netzwerks:
- **[devkitc-gps-oled](https://github.com/frawi-mfa/devkitc-gps-oled)** — Mobiler GPS Monitor
- **[esp32c3-bme280-ble](https://github.com/frawi-mfa/esp32c3-bme280-ble)** — BLE Wetterstation Nodes
