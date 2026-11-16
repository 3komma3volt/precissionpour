#define FIRMWARE_VERSION "1.0.0"

#define DISABLE_ALL_LIBRARY_WARNINGS
#include <lvgl.h>
#include <ui.h>
#include <Arduino.h>
#include "lv_drv_conf.h"
#include "driver_if.h"
#include "game_functions.h"
#include "sounds.h"

#include <HX711_ADC.h>
#include <EEPROM.h>

const int LOADCELL_DOUT_PIN = 22;
const int LOADCELL_SCK_PIN = 27;


HX711_ADC LoadCell(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

SemaphoreHandle_t guiMutex;
QueueHandle_t weightQueue = NULL;

void Task_LVGL(void *pvParameters);
void Task_WeightCell(void *pvParameters);

void setup()
{
  Serial.begin(115200);
  lv_init();
  Serial.println("Initializing...");

  guiMutex = xSemaphoreCreateMutex();
  if (guiMutex == NULL)
  {
    // Handle semaphore creation failure
    Serial.println("semaphore creation failure");
    return;
  }

  // LVGL display driver stup
  display_drv_init();
  static lv_disp_draw_buf_t draw_buf;

  static lv_color_t *disp_draw_buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * LV_DRAW_BUFFER_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!disp_draw_buf)
  {
    Serial.println("LVGL disp_draw_buf allocate failed!");
  }
  else
  {
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, LV_DRAW_BUFFER_SIZE);
    /* Initialize the display */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = CONFIG_LVGL_DISPLAY_WIDTH;
    disp_drv.ver_res = CONFIG_LVGL_DISPLAY_HEIGHT;
    disp_drv.flush_cb = display_drv_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    lv_obj_clean(lv_scr_act());
  }

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touch_drv_read;
  lv_indev_drv_register(&indev_drv);

  ui_init();
  lv_label_set_text(ui_lblVersion, FIRMWARE_VERSION);

  uint8_t gameMode = GAME_NONE;

  weightQueue = xQueueCreate(2, sizeof(float));
  if (weightQueue == NULL)
  {
    Serial.println("Failed to create queue!");
    while (1)
      ;
  }

  soundSetup();

  xTaskCreatePinnedToCore(Task_LVGL,   // Pointer to the task entry function.
                          "Task_LVGL", // A descriptive name for the task.
                          1024 * 10,   // The size of the task stack specified as the number of bytes
                          NULL,        // Pointer that will be used as the parameter for the task being created.
                          3,           // The priority at which the task should run.
                          NULL,        // Used to pass back a handle by which the created task can be referenced.
                          0);          // The core to which the task is pinned to, or tskNO_AFFINITY if the task has no core affinity.

  xTaskCreatePinnedToCore(Task_WeightCell,
                          "Task_WeightCell",
                          1024 * 3,
                          NULL,
                          2,
                          NULL,
                          1);


  LoadCell.begin();
  LoadCell.start(500, true);
  if (LoadCell.getTareTimeoutFlag())
  {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  }
  else
  {
    EEPROM.begin(10);

    float calibrationValue = EEPROM.readFloat(0);
    if (isnan(calibrationValue))
    {
      calibrationValue = 1.0;
    }

    LoadCell.setCalFactor(calibrationValue);
    LoadCell.refreshDataSet();
  }
  Serial.println("Setup complete");
  lv_obj_add_flag(ui_lblWait, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(ui_btnSetting, LV_OBJ_FLAG_HIDDEN);
}

void Task_LVGL(void *pvParameters)
{
  unsigned long lastTickMillis = 0;
  unsigned long lastUIUpdate = 0;
  while (1)
  {
    unsigned int tickPeriod = millis() - lastTickMillis;

    lastTickMillis = millis();

    if (xSemaphoreTake(guiMutex, portMAX_DELAY) == pdTRUE)
    {
      lv_timer_handler();

      if (millis() - lastUIUpdate > 100)
      {
        updateCurrentScreen();
        lastUIUpdate = millis();
      }

      xSemaphoreGive(guiMutex);
    }

    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void Task_WeightCell(void *pvParameters)
{

  while (1)
  {
    static boolean newDataReady = 0;

    if (LoadCell.update())
      newDataReady = true;

    if (newDataReady)
    {

      float i = LoadCell.getData();
      xQueueSend(weightQueue, &i, portMAX_DELAY);
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}





void loop()
{
}
