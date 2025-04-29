#include <Arduino.h>
#include <Ticker.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>
#include <Arduino_DriveBus_Library.h>
#include "pin_config.h"
#include <lvgl.h>

#include "PageManager.h"
#include "HomePage.h"
#include "GpsManager.h"

// display
static Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3
);
static Arduino_GFX *gfx = new Arduino_SH8601(
  bus, -1, 0, false, LCD_WIDTH, LCD_HEIGHT
);

// touch
static std::shared_ptr<Arduino_HWIIC> i2c_bus;
static std::unique_ptr<Arduino_IIC>   touchDev;
void IRAM_ATTR touchIRQ() {
  if(touchDev) touchDev->IIC_Interrupt_Flag = true;
}

// lvgl
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[LCD_WIDTH * 40];
static Ticker    lvTicker;

void my_disp_flush(lv_disp_drv_t *d, const lv_area_t *a, lv_color_t *c) {
  int w = a->x2 - a->x1 + 1, h = a->y2 - a->y1 + 1;
  gfx->startWrite();
  gfx->draw16bitRGBBitmap(a->x1, a->y1, (uint16_t*)c, w, h);
  gfx->endWrite();
  lv_disp_flush_ready(d);
}

void my_touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  if(touchDev->IIC_Interrupt_Flag) {
    touchDev->IIC_Interrupt_Flag = false;
    int16_t x = touchDev->IIC_Read_Device_Value(
      Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X
    );
    int16_t y = touchDev->IIC_Read_Device_Value(
      Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y
    );
    data->state   = LV_INDEV_STATE_PR;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

void setup() {
  Serial.begin(115200);
  // display init
  gfx->begin();
  gfx->setRotation(0);
  gfx->Display_Brightness(200);
  // touch init
  Wire.begin(IIC_SDA, IIC_SCL);
  i2c_bus = std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);
  touchDev.reset(new Arduino_FT3x68(
    i2c_bus, FT3x68_DEVICE_ADDRESS, TOUCH_DRIVEBUS_DEFAULT, TP_INT, touchIRQ
  ));
  while(!touchDev->begin()) {
    Serial.println("Waiting for touch...");
    delay(200);
  }
  // lvgl init
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, nullptr, LCD_WIDTH*40);
  {
    static lv_disp_drv_t dd; lv_disp_drv_init(&dd);
    dd.hor_res = LCD_WIDTH; dd.ver_res = LCD_HEIGHT;
    dd.flush_cb = my_disp_flush; dd.draw_buf = &draw_buf;
    lv_disp_drv_register(&dd);
  }
  {
    static lv_indev_drv_t id; lv_indev_drv_init(&id);
    id.type = LV_INDEV_TYPE_POINTER; id.read_cb = my_touchpad_read;
    lv_indev_drv_register(&id);
  }
  lvTicker.attach_ms(5, []{ lv_tick_inc(5); });
  // GPS init
  GpsManager::instance().begin(
    &Serial1, 
    9600, 
    GPS_RX, 
    GPS_TX,
    PMTK_SET_NMEA_OUTPUT_ALLDATA,  // <— enable GGA, GSA, GSV, RMC, etc.
    PMTK_SET_NMEA_UPDATE_1HZ
  );
  // show home
  PageManager::instance().pushPage(new HomePage());
}

void loop() {
  GpsManager::instance().update();
  lv_timer_handler();
  delay(5);
}
