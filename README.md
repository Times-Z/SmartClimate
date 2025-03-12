# SmartClimate

SmartClimate is an IoT-based weather monitoring and smart device tracking system using the ESP32-C6 microcontroller with an ST7789 172×320 TFT display. This project provides real-time environmental data and smart home device status, making it an efficient tool for monitoring climate conditions and IoT devices.

## Prerequisites
Before setting up SmartClimate, ensure you have the following installed:

### Software Requirements
- **Python 3** + `pip`
- **VS Code** with the following extensions:
  - [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
  - [ESP-IDF (latest version)](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension)

### Hardware Requirements
- **ESP32-C6** microcontroller
- **ST7789 TFT display** (172×320 resolution)
- Environmental sensors (optional, e.g., DHT22, BMP280, etc.)
- Power supply (USB-C or battery)

### Tester hardware
[Aliexpress ESP32-C6 + TFT 1.47" screen](https://fr.aliexpress.com/item/1005008137447784.html?spm=a2g0o.order_list.order_list_main.16.35e25e5bMBmZyY&gatewayAdapt=glo2fra)

### Partitions

| # Name                                            |      Type                                        |  SubType   |  Offset   |    Size   |    Flags |
|:--------------------------------------------------|:-------------------------------------------------|:-----------|:----------|:----------|---------:|
| nvs                                               | data                                             | nvs        | 0x9000    | 0x6000    |      nan |
| factory                                           | 0                                                | 0          | 0x10000   | 2M        |      nan |
| flash_test                                        | data                                             | fat        |           | 528K      |      nan |

## Installation
1. **Clone the repository:**
   ```sh
   git clone https://github.com/your-repo/SmartClimate.git
   cd SmartClimate
   ```
2. **Configure ESP-IDF:**
   - Open VS Code and install the ESP-IDF extension.
   - Follow the setup instructions to configure the ESP32-C6 environment.
3. **Run the dev container with vscode extension**
4. **Build and flash the firmware:**
   ```sh
   idf.py build
   idf.py flash
   ```

## Usage
- Power on the ESP32-C6 with the TFT screen attached.
- The device will connect to the configured Wi-Fi network and start displaying weather data and smart device statuses.
- Use the serial monitor for debugging:
  ```sh
  idf.py monitor
  ```

## Roadmap
- [ ] First release

## License
This project is licensed under the MIT License.

## Contributions
Contributions are welcome! Feel free to submit a pull request or open an issue for discussion.

---
Happy Coding! 😊
