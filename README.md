# SmartClimate

> Little weather station

<div align="center">
  <br/>

[![Latest Release](https://img.shields.io/github/v/release/Times-Z/smartclimate?label=Latest%20Version&color=c56a90&style=for-the-badge&logo=star)](https://github.com/Times-Z/smartclimate/releases)
[![Build Status](https://img.shields.io/github/actions/workflow/status/Times-Z/smartclimate/.github/workflows/build.yml?branch=main&label=Pipeline%20Status&color=c56a90&style=for-the-badge&logo=star)](https://github.com/Times-Z/smartclimate/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](LICENSE)

  <br/>

**SmartClimate** is an IoT-based weather monitoring and smart device tracking system using the ESP32-C6 microcontroller with an ST7789 172×320 TFT display. This project provides real-time environmental data and smart home device status, making it an efficient tool for monitoring climate conditions and IoT devices.

  <br/>

<table>
  <tr>
  </tr>
</table>

</div>

---

## Table of contents

- [Prerequisites](#-prerequisites)
- [Hardware setup](#-hardware-setup)
- [Installation](#-installation)
- [Configuration](#-configuration)
- [API documentation](#-api-documentation)
- [Features](#-features)
- [Contributing](#-contributing)
- [License](#-license)

---

## Prerequisites

### Software requirements

#### Using devcontainer (zero setup)

The project includes a **pre-configured devcontainer** with everything pre-installed:

- Python 3.12
- ESP-IDF 5.5+
- ESP32 toolchain
- VS Code extensions
- Build tools

**Simply clone and open:**

```bash
git clone https://github.com/Times-Z/smartclimate.git
code smartclimate
# Click "Reopen in Container" when prompted
```

All dependencies are automatically available inside the container \o/

#### Manual setup

If you prefer not to use devcontainer, install manually:

| Tool        | Version | Purpose                     |
| ----------- | ------- | --------------------------- |
| **Python**  | 3.12+   | Build system dependency     |
| **ESP-IDF** | 5.5+    | ESP32 development framework |

Then configure ESP-IDF following the [official guide](https://docs.espressif.com/projects/esp-idf/en/v5.5.2/esp32/get-started/index.html#manual-installation)

---

## Hardware setup

### Required components

| Component           | Model   |
| ------------------- | ------- |
| **Microcontroller** | ESP32c6 |

---

## Flash memory layout

The firmware uses a custom partition scheme optimized for storage and performance:

| Partition | Type | SubType | Offset   | Size    | Purpose                     |
| :-------- | :--- | :------ | :------- | :------ | :-------------------------- |
| `nvs`     | data | nvs     | 0x9000   | 512 KB  | Configuration & credentials |
| `factory` | app  | factory | 0x90000  | 2048 KB | Main firmware binary        |
| `config`  | data | spiffs  | 0x290000 | 128 KB  | JSON settings files         |
| `www`     | data | spiffs  | 0x2B0000 | 1344 KB | Web interface assets        |

---

## Installation

### Step 1: clone the repository

```bash
git clone https://github.com/Times-Z/smartclimate.git
cd smartclimate
```

### Step 2: set up ESP-IDF

**Option A: Using VS code extension**

1. Install the **ESP-IDF** extension in VS Code
2. Follow the extension's setup wizard
3. Select ESP-IDF v5.5+

**Option B: Using devcontainer**

No setup needed! The devcontainer handles everything

### Step 3: Configure the project

#### Method 1: JSON configuration (boot-time)

Create your config file:

```bash
cp main/config/default.json main/config/config.json
```

Edit `main/config/config.json`:

```json
{
  "wifi_ssid": "Your_WiFi_Network",
  "wifi_password": "Your_WiFi_Password",
  "api_key": "your_secure_api_key",
  "ntp_server": "pool.ntp.org"
}
```

**Note:** This file is used only on first boot. Afterward, configuration persists in NVS

#### Method 2: Web UI / REST API

Once the device boots:

1. Connect to wifi or access the captive portal
2. Open the web dashboard
3. Configure settings through the UI
4. Changes are saved automatically to NVS

#### WiFi access point mode

If WiFi credentials are missing or invalid:

- **SSID:** `SmartClimate`
- **Password:** `$tr0ngWifi`
- **Portal:** Auto-opens on compatible devices (iOS/Android), otherwise go to `http://10.0.1.1`

---

## Build & flash

### Build the firmware

```bash
idf.py build
```

### Flash to ESP32

```bash
idf.py flash
```

### Monitor serial output

```bash
idf.py monitor
```

**Note:** All of this commands can be used at the same time, eg : `idf.py build flash monitor`

---

## API documentation

Complete API endpoints are documented in the [Swagger/OpenAPI specification](./swagger.yml)

---

## Features

### Implemented

- [x] REST API with HTTP endpoints
- [x] WiFi Access Point (AP) mode with captive portal
- [x] WiFi Station (STA) mode with auto-connect
- [x] Screen display QR code to connect
- [x] Screen display version information
- [x] Screen display gif
- [x] JSON configuration files
- [x] Persistent NVS storage (credentials & settings)
- [x] System uptime tracking
- [x] API key authentication (X-API-Key header)
- [x] NTP time synchronization
- [x] Log streaming via web UI

### Planned

- [] Gif better display
- [] Clock display
- [] Wheater informations

---

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details

---

## Support

- Check the [Swagger API documentation](./swagger.yml) for endpoint details
- Found a bug? [Open an issue](https://github.com/Times-Z/light-bar-2-api/issues)
- Have questions? [Start a discussion](https://github.com/Times-Z/light-bar-2-api/discussions)

---

<div align="center">

**Made with ❤️**

[Star us on GitHub](https://github.com/Times-Z/light-bar-2-api) if you find this project useful!

</div>
