# Golf GPS Device

A DIY handheld golf GPS unit built on ESP32-S3 with a 1.8″ Touch-AMOLED display and LVGL UI. Shows hole layouts, distances to front/center/back, hazards, and (optionally) swing tracking via IMU.

---

## Table of Contents

1. [Features](#features)  
2. [Hardware Requirements](#hardware-requirements)  
3. [Software Requirements](#software-requirements)  
4. [Wiring & Pinout](#wiring--pinout)  
5. [Installation & Build](#installation--build)  
6. [Course Data](#course-data)  
7. [Usage](#usage)  
8. [Project Structure](#project-structure)  
9. [Customization & Extending](#customization--extending)  
10. [Contributing](#contributing)  
11. [License](#license)  
12. [Contact](#contact)  

---

## Features

- **Hole View**: Front / center / back yardages  
- **Hazard Overlay**: Bunkers, water hazards  
- **IMU Tracking**: Swing tempo & path (optional)  
- **Course Selection**: Built-in or custom courses  
- **LVGL Touch UI**: Smooth menus, pinch-zoom  
- **Low Power Sleep Mode**  

---

## Hardware Requirements

| Component           | Description / Vendor                 |
|---------------------|--------------------------------------|
| **Microcontroller** | ESP32-S3 Feather (or equivalent)     |
| **Display & Touch** | 1.8″ SPI AMOLED + capacitive touch   |
| **GPS Module**      | Adafruit Ultimate GPS @ 5 Hz         |
| **IMU (optional)**  | QMI8658 or MPU-6050                  |
| **Power**           | 3.7 V LiPo + charger + USB-C         |
| **Misc**            | Wires, standoffs, enclosure          |

Available from:  
- [Adafruit](https://www.adafruit.com/)  
- [SparkFun](https://www.sparkfun.com/)  
- [AliExpress / eBay](https://www.aliexpress.com/)  

---

## Software Requirements

- **Arduino IDE** ≥ 2.0  
- **Board Package**: Espressif ESP32  
- **Libraries**:
  - `lvgl`
  - `Arduino_GFX_Library`
  - `Arduino_DriveBus_Library`
  - `Ticker`
  - `Adafruit_GPS`
  - `Wire`

Install via Library Manager or:

```bash
cd ~/Arduino/libraries
git clone https://github.com/lvgl/lvgl.git
git clone https://github.com/moononournation/Arduino_GFX.git
```

---

## Wiring & Pinout

| Signal         | ESP32-S3 Pin | Module / Display Pin |
|---------------:|-------------:|---------------------:|
| LCD_SCLK       | GPIO 18      | SCLK                  |
| LCD_MOSI       | GPIO 23      | SDI                   |
| LCD_CS         | GPIO 5       | CS                    |
| LCD_DC         | GPIO 2       | DC                    |
| LCD_RESET      | GPIO 4       | RST                   |
| TOUCH_IRQ      | GPIO 33      | INT                   |
| TOUCH_SDA      | GPIO 21      | SDA                   |
| TOUCH_SCL      | GPIO 22      | SCL                   |
| GPS_TX         | GPIO 16      | RX                    |
| GPS_RX         | GPIO 17      | TX                    |
| IMU_I²C SDA    | GPIO 21      | SDA                   |
| IMU_I²C SCL    | GPIO 22      | SCL                   |
| BATTERY+       | VIN (USB-C)  | LiPo +                |
| GND            | GND          | GND                   |

See `pin_config.h` for definitions.

---

## Installation & Build

1. **Clone Repo**
   ```bash
   git clone https://github.com/yourusername/golf-gps.git
   cd golf-gps
   ```
2. **Configure**
   - Edit `pin_config.h` if your wiring differs.  
   - Tweak `GPS_TICK_MS` and `IMU_TICK_MS` in `golf-gps.ino`.
3. **Compile & Upload**
   - Select **ESP32-S3 Feather** in Arduino IDE.  
   - Upload via USB-C.
4. **First Run**
   - Power on, tap **Courses** to select.  
   - Wait ~30 s for GPS lock outdoors.

---

## Course Data

- **Built-in**: Provided in `courses_data.h` for Centurion & Irene.  
- **Custom**: Use Overpass API to extract OSM data:

  ```overpass
  [out:json][timeout:25];
  area["name"="Centurion Golf Course"]->.a;
  (
    way(area.a)["leisure"="golf_course"];
    relation(area.a)["leisure"="golf_course"];
  );
  out body;
  >;
  out skel qt;
  ```

- Convert the JSON output into a C header with `scripts/osm2courses.py`.

---

## Usage

- **Home Screen**  
  - Tap **Courses** → list of courses.
- **Hole View**  
  - Shows hole number, par, yardages.  
  - Swipe left/right to change hole.
- **IMU Mode**  
  - Tap **Swing** and follow prompts.
- **Sleep**  
  - Hold power button for 3 s to sleep; press again to wake.

---

## Project Structure

```
golf-gps.ino         # Main Arduino sketch
pin_config.h         # Pin mappings
CoursesManager.*     # Course loading & management
GpsManager.*         # GPS parsing
IMUManager.*         # IMU data fusion
PageManager.*        # UI navigation
HomePage.*           # Main menu UI
CoursesPage.*        # Course list UI
LocationPage.*       # Map & distance UI
Layout.*             # LVGL styles & layouts
courses_data.h       # Generated course data
scripts/
└─ osm2courses.py    # OSM → C header conversion tool
```

---

## Customization & Extending

- **Add Pages**: Create a new `Page` subclass in `PageManager`.  
- **UI Theme**: Modify LVGL styles in `Layout.cpp`.  
- **New Sensors**: Integrate and implement in `IMUManager.cpp`.  
- **Localization**: Externalize strings and use a lookup table.

---

## Contributing

1. Fork & clone.  
2. Create a branch: `git checkout -b feature/my-feature`.  
3. Commit, push & open a PR.  
4. Ensure it compiles and displays correctly on the 1.8″ screen.

---

## License

Distributed under the **MIT License**. See [LICENSE](./LICENSE) for details.

---

## Contact

Darren Louw · [@yourhandle](https://github.com/yourusername)  
Project: https://github.com/yourusername/golf-gps  
