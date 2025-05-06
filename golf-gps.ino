// golf-gps.ino
#include <Arduino.h>
#include <Wire.h>
#include <Ticker.h>                 // for lv_tick, GPS & IMU ticks
#include <Arduino_GFX_Library.h>
#include <Arduino_DriveBus_Library.h>
#include <lvgl.h>

#include "pin_config.h"             // LCD_*, TP_INT, IIC_SDA/SCL
#include "Layout.h"
#include "PageManager.h"
#include "HomePage.h"
#include "GpsManager.h"
#include "CoursesManager.h"
#include "IMUManager.h"
#include "SensorFusion.h"

// ─── CONFIG ────────────────────────────────────────────────────────────────
static constexpr uint32_t LV_TICK_PERIOD_MS = 1;    // LVGL 1 ms tick
static constexpr uint16_t DRAW_BUF_HEIGHT    = 80; // LVGL buffer lines

static constexpr uint32_t GPS_TICK_MS = 200;  // GPS @ ~5 Hz
static constexpr uint32_t IMU_TICK_MS =   5;  // IMU @ ~200 Hz

// ─── HARDWARE OBJECTS ──────────────────────────────────────────────────────
static Ticker lvglTicker;
static Ticker gpsTicker;
static Ticker imuTicker;

static Arduino_DataBus *bus   = nullptr;
static Arduino_GFX     *gfx   = nullptr;
static std::shared_ptr<Arduino_HWIIC> i2c_bus;
static std::unique_ptr<Arduino_IIC>   touchDev;

// LVGL draw buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t draw_buf_pixels[LCD_WIDTH * DRAW_BUF_HEIGHT];

// ─── FORWARD DECLARATIONS ──────────────────────────────────────────────────
static void lvglTick();
static void dispFlush(lv_disp_drv_t*, const lv_area_t*, lv_color_t*);
static void touchISR();
static void touchRead(lv_indev_drv_t*, lv_indev_data_t*);
static void onImuTick();
static void onGpsTick();

static void initSerial();
static void initLVGL();
static void initDisplay();
static void initTouch();
static void initGPS();
static void initIMU();

// ─── IMPLEMENTATION ────────────────────────────────────────────────────────

// LVGL tick callback
static void lvglTick() {
  lv_tick_inc(LV_TICK_PERIOD_MS);
}

// Display flush callback
static void dispFlush(lv_disp_drv_t *drv,
                      const lv_area_t *area,
                      lv_color_t *pixels) {
  int w = area->x2 - area->x1 + 1;
  int h = area->y2 - area->y1 + 1;
  gfx->startWrite();
  gfx->draw16bitRGBBitmap(area->x1, area->y1,
                          (uint16_t*)pixels, w, h);
  gfx->endWrite();
  lv_disp_flush_ready(drv);
}

// Touch IRQ
static void touchISR() {
  if (touchDev) touchDev->IIC_Interrupt_Flag = true;
}

// Touch read callback
static void touchRead(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  if (touchDev->IIC_Interrupt_Flag) {
    touchDev->IIC_Interrupt_Flag = false;
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchDev->IIC_Read_Device_Value(
      Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
    data->point.y = touchDev->IIC_Read_Device_Value(
      Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

// IMU tick: read accel, run EKF predict
static void onImuTick() {
  static uint32_t lastUs = micros();
  uint32_t now = micros();
  float dt = (now - lastUs) * 1e-6f;
  lastUs = now;

  IMUManager::instance().update();
  auto raw = IMUManager::instance().getRaw();
  SensorFusion::instance().predict(raw.ax, raw.ay, dt);
}

// GPS tick: read fix, run EKF update
static void onGpsTick() {
  GpsManager::instance().update();
  auto d = GpsManager::instance().getData();
  if (d.fix) {
    SensorFusion::instance().updateGPS(d.lat, d.lon);
  }
}

// Serial for debug
static void initSerial() {
  Serial.begin(115200);
}

// Initialize LVGL
static void initLVGL() {
  lv_init();
  lvglTicker.attach_ms(LV_TICK_PERIOD_MS, lvglTick);

  lv_disp_draw_buf_init(&draw_buf, draw_buf_pixels, nullptr,
                        LCD_WIDTH * DRAW_BUF_HEIGHT);

  static lv_disp_drv_t dd;
  lv_disp_drv_init(&dd);
  dd.hor_res = LCD_WIDTH;
  dd.ver_res = LCD_HEIGHT;
  dd.flush_cb = dispFlush;
  dd.draw_buf = &draw_buf;
  lv_disp_drv_register(&dd);

  static lv_indev_drv_t id;
  lv_indev_drv_init(&id);
  id.type = LV_INDEV_TYPE_POINTER;
  id.read_cb = touchRead;
  lv_indev_drv_register(&id);
}

// Initialize screen
static void initDisplay() {
  bus = new Arduino_ESP32QSPI(
    LCD_CS, LCD_SCLK,
    LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);
  gfx = new Arduino_SH8601(
    bus, -1, 0, false, LCD_WIDTH, LCD_HEIGHT);
  gfx->begin();
  gfx->setRotation(0);
  gfx->Display_Brightness(200);
}

// Initialize touchscreen
static void initTouch() {
  pinMode(TP_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TP_INT),
                  touchISR, FALLING);

  Wire.begin(IIC_SDA, IIC_SCL);
  i2c_bus = std::make_shared<Arduino_HWIIC>(
    IIC_SDA, IIC_SCL, &Wire);
  touchDev.reset(new Arduino_FT3x68(
    i2c_bus,
    FT3x68_DEVICE_ADDRESS,
    TOUCH_DRIVEBUS_DEFAULT,
    TP_INT,
    touchISR));
  while (!touchDev->begin()) {
    Serial.println("Waiting for touch...");
    delay(200);
  }
}

// Initialize GPS + ticker
static void initGPS() {
  GpsManager::instance().begin(
    &Serial1, 9600,
    GPS_RX, GPS_TX,
    PMTK_SET_NMEA_OUTPUT_ALLDATA,
    PMTK_SET_NMEA_UPDATE_5HZ);

  gpsTicker.attach_ms(GPS_TICK_MS, onGpsTick);
}

// Initialize IMU + ticker
static void initIMU() {
  if (!IMUManager::instance().begin()) {
    Serial.println("IMU init failed!");
    while(true);
  }
  IMUManager::instance().calibrate();
  imuTicker.attach_ms(IMU_TICK_MS, onImuTick);
}

// ─── ARDUINO HOOKS ─────────────────────────────────────────────────────────
void setup() {
  initSerial();

  // set initial EKF covariances
  SensorFusion::instance().initCov();

  initDisplay();
  initTouch();
  initLVGL();

  initGPS();
  initIMU();

  CoursesManager::instance().beginFromFlash();
  PageManager::instance().pushPage(new HomePage());
}

void loop() {
  // pump LVGL
  lv_timer_handler();
  // delay(2);

  // optionally, print fused state every second
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    float lat, lon;
    SensorFusion::instance().getFusedLatLon(lat, lon);
    if (!isnan(lat)) {
      Serial.printf("Golf position: %.8f, %.8f\n", lat, lon);
    }
  }
}
