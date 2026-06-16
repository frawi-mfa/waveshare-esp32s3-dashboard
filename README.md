# Waveshare ESP32-S3 Network Dashboard

Netzwerk-Dashboard auf Basis des Waveshare ESP32-S3-Touch-LCD-4.3 mit 
4.3" Touch Display (800×480). Zeigt Netzwerkstatus, Ping-Monitor, 
NTP Uhrzeit und WiFi Informationen.

## Hardware

| Komponente | Details |
|---|---|
| **Board** | Waveshare ESP32-S3-Touch-LCD-4.3 |
| **Display** | 4.3" IPS RGB, 800×480 Pixel, kapazitiver Touch |
| **Chip** | ESP32-S3-WROOM-1, 240MHz, 16MB Flash |
| **Interfaces** | I2C, CAN, RS-485, Sensor AD, TF-Card |

## Features

### Seite 1 — Netzwerk Dashboard
- 📡 SSID und IP-Adresse
- 📶 WiFi Signalstärke (RSSI)
- 🔌 MAC-Adresse
- 📅 Datum und Uhrzeit (NTP, deutsche Zeitzone)

### Seite 2 — Ping Monitor
- 🟢 Fritz-Box (192.168.1.1)
- 🟢 Tempel2 File Server (192.168.1.50)
- 🔵 Fritz Repeater 1 (192.168.1.33)
- 🟢 Fritz Repeater 2 (192.168.1.131)
- 🟢 Agfeo Telefonanlage (192.168.1.40)

Automatischer Seitenwechsel alle 10 Sekunden.

## Software

- **Framework:** Arduino (PlatformIO)
- **Platform:** espressif32@^6.11.0
- **Libraries:**
  - moononournation/GFX Library for Arduino@1.4.9
  - bitbank2/bb_captouch
  - marian-craciunescu/ESP32Ping

## Wichtige Hinweise

### Display Pins (RGB Interface)
DE=5, VSYNC=3, HSYNC=46, PCLK=7
R: 14,38,18,17,10
G: 39,0,45,48,47,21
B: 1,2,42,41,40

### Konfiguration
Credentials und Geräteliste werden in `src/config.h` konfiguriert 
(nicht im Repo — siehe `src/config.h.example`):

```cpp
// WiFi
#define WIFI_SSID     "dein_netzwerk"
#define WIFI_PASSWORD "dein_passwort"

// Ping Geräte
#define PING_DEVICES { \
    {"192.168.1.1", "Fritz-Box"} \
}
#define PING_DEVICE_COUNT 1
```

## Installation

```bash
git clone https://github.com/frawi-mfa/waveshare-esp32s3-dashboard.git
cd waveshare-esp32s3-dashboard
cp src/config.h.example src/config.h
# config.h anpassen
# In PlatformIO öffnen und flashen
```

## Bekannte Probleme

- Die offiziellen Waveshare Demo Libraries sind nicht mit PlatformIO 
  kompatibel (C++17/C++20 Konflikte)
- Lösung: Arduino_GFX Library von moononournation funktioniert einwandfrei
- PSRAM Warnung beim Start ist harmlos

## Geplante Erweiterungen

- BME280 Wetterstation (I2C Header)
- BLE Empfang von ESP32-C3 Sensor Nodes
- Touch Navigation mit Hauptmenü
- BLE Scanner
- Fritzbox API Integration
- Außentemperatur vom Balkon Sensor

## Projektkontext

Teil eines größeren ESP32 Projektnetzwerks:
- **[devkitc-gps-oled](https://github.com/frawi-mfa/devkitc-gps-oled)** — Mobiler GPS Monitor
- **[esp32c3-bme280-ble](https://github.com/frawi-mfa/esp32c3-bme280-ble)** — Wetterstation Node