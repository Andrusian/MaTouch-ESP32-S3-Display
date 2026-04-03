# MaTouch Base

Baseline application for MaTouch ESP32-S3 3.5" (ILI9488 / FT6236).

## Hardware
- ESP32-S3-WROOM-1-N16R2 (16MB Flash, 2MB OPI PSRAM)
- ILI9488 SPI display, 480×320
- FT6236 capacitive touch (I2C)
- CP2104 USB-UART bridge

## Project phases
| Phase | Description | Status |
|-------|-------------|--------|
| 1     | WiFi provisioning (captive portal + Preferences) | ✅ |
| 1b    | NTP time sync | ✅ |
| 2     | Fractal renderer with touch zoom | TODO |
| 3     | JPEG download + display | TODO |

## First boot
1. Flash via the **CP2104 USB port** (left-hand USB-C when in landscape).
2. On first boot, device starts AP: **"MaTouch-Setup"**
3. Connect your phone/laptop to that AP → browser opens captive portal
4. Enter your WiFi SSID/password → device connects and saves to NVS
5. Subsequent boots connect automatically

## Pin assignments (verify against your schematic revision)

### Display (SPI2)
| Signal | GPIO |
|--------|------|
| SCLK   | 12   |
| MOSI   | 11   |
| MISO   | 13   |
| DC     | 10   |
| CS     | 9    |
| RST    | —    |

### Touch (I2C0)
| Signal | GPIO |
|--------|------|
| SDA    | 38   |
| SCL    | 39   |
| INT    | 40   |

### Backlight
- **TBD** — confirm pin from schematic, then uncomment the Light_PWM
  block in `LovyanGFX_config.h`. Placeholder: GPIO 45.

## OTA updates
Once on WiFi, you can upload wirelessly:
```
pio run -t upload --upload-port <device-ip>
```
Or set `upload_port` in `platformio.ini`.

## Partition layout (16MB)
| Partition | Size   | Purpose            |
|-----------|--------|--------------------|
| nvs       | 20kB   | WiFi creds, prefs  |
| app0      | ~6MB   | Active firmware    |
| app1      | ~6MB   | OTA target         |
| spiffs    | 4MB    | Future: assets     |
| coredump  | 64kB   | Crash capture      |

## Known issues / watch-outs
- **Do not** set `board_build.arduino.memory_type = qio_opi` — causes
  crash-reboot loop on N16R2. Let framework auto-detect PSRAM.
- Native USB CDC needs `delay(2000)` before first `Serial.begin()`.
- `brltty` on Debian will steal the CP2104 device. Fix:
  `sudo apt remove brltty`
