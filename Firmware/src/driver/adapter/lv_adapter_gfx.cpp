#include <lvgl.h>
#include <ui.h>
#include <Arduino.h>
#include "lv_drv_conf.h"
#include "../../sounds.h"


#include <TFT_eSPI.h>
#include <TFT_Touch.h>

#define TFT_PWM_CHANNEL_BL 7
#define TFT_PWM_FREQ_BL 5000
#define TFT_PWM_BITS_BL 8
#define TFT_PWM_MAX_BL ((1 << TFT_PWM_BITS_BL) - 1)
#define TFT_BL 21

TFT_eSPI tft = TFT_eSPI();
TFT_Touch touch = TFT_Touch(XPT2046_CS, XPT2046_CLK, XPT2046_MOSI, XPT2046_MISO);


/* Display flushing */
void display_drv_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);


    uint32_t wh = w * h;
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    while (wh--)
      tft.pushColor(color_p++->full);


    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void touch_drv_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;

    data->state = LV_INDEV_STATE_REL;
    

    if (touch.Pressed()) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x =  touch.X();
        data->point.y = touch.Y();
    
    }

}

void display_drv_init()
{

    tft.init();
    tft.setRotation(1);
    touch.setRotation(1);
    touch.setCal(498, 3415, 746, 3560, 320, 240, 1);

   pinMode(TFT_BL, OUTPUT);
   digitalWrite(TFT_BL, HIGH);
   ledcSetup(TFT_PWM_CHANNEL_BL, TFT_PWM_FREQ_BL, TFT_PWM_BITS_BL);
   ledcAttachPin(TFT_BL, TFT_PWM_CHANNEL_BL);
   ledcWrite(TFT_PWM_CHANNEL_BL, TFT_PWM_MAX_BL);


#ifdef SCREEN_ROTATION    
    gfx.setRotation(SCREEN_ROTATION);
#endif
}

