#include <lvgl.h>
#include <Ticker.h>
#include <TFT_eSPI.h>

#include <Wire.h>
#include <Adafruit_FT6206.h>
// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ts = Adafruit_FT6206();

#define LVGL_TICK_PERIOD 20

Ticker tick; /* timer for interrupt handler */
TFT_eSPI tft = TFT_eSPI(); /* TFT instance */

#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{

  Serial.printf("%s@%d->%s\r\n", file, line, dsc);
  delay(100);
}
#endif

/* Display flushing */
void disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *color_p) {
  uint16_t c;
  tft.startWrite(); /* Start new TFT transaction */
  tft.setAddrWindow(x1, y1, (x2 - x1 + 1), (y2 - y1 + 1)); /* set the working window */
  for (int y = y1; y <= y2; y++) {
    for (int x = x1; x <= x2; x++) {
      c = color_p->full;
      tft.writeColor(c, 1);
      color_p++;
    }
  }
  tft.endWrite(); /* terminate TFT transaction */
  lv_flush_ready(); /* tell lvgl that flushing is done */
}

/* Interrupt driven periodic handler */
static void lv_tick_handler(void)
{

  lv_tick_inc(LVGL_TICK_PERIOD);
}

static lv_res_t btn_click_action(lv_obj_t * btn)
{
    uint8_t id = lv_obj_get_free_num(btn);

    printf("Button %d is released\n", id);

    /* The button is released.
     * Make something here */

    return LV_RES_OK; /*Return OK if the button is not deleted*/
}

static bool ts_input_read(lv_indev_data_t* data)
{
    if (ts.touched())
    {   
      // Retrieve a point  
      TS_Point p = ts.getPoint(); 
      // rotate coordinate system
      // flip it around to match the screen.
     // p.y = map(p.x, 0, 320, 320, 0);
      //p.x = map(p.y, 0, 480, 480, 0);
      //int y = tft.height() - p.x;
      //int x = p.y;
    data->point.x = map(p.y, 0, 480, 0, 480);
    data->point.y = map(p.x, 0, 320, 320, 0);
    data->state = LV_INDEV_STATE_PR;
    }
    return false;
}


void setup() {

  Serial.begin(115200); /* prepare for possible serial debug */

  lv_init();

#if USE_LV_LOG != 0
  lv_log_register_print(my_print); /* register print function for debugging */
#endif

  tft.begin(); /* TFT init */
  tft.setRotation(1); /* Landscape orientation */

  ts.begin(40); /* Touchscreen init */
  
  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.disp_flush = disp_flush;
  lv_disp_drv_register(&disp_drv);


  /*Initialize the touch pad*/
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read = ts_input_read; /*Library ready your touchpad via this function*/
  lv_indev_drv_register(&indev_drv);

  /*Initialize the graphics library's tick*/
  tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);

  /* Create simple label */
  lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_pos(label, 10, 10);
  lv_label_set_text(label, "LittlevGL on ESP32 with TFT350-480x320 CTP");
  //lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

  static lv_style_t style_shadow;
  lv_style_copy(&style_shadow, &lv_style_pretty);
  style_shadow.body.shadow.width = 4;
  style_shadow.body.radius = LV_RADIUS_CIRCLE;

  /*Create a normal button*/
  lv_obj_t * btn1 = lv_btn_create(lv_scr_act(), NULL);
  //lv_cont_set_fit(btn1, true, true); /*Enable resizing horizontally and vertically*/
  //lv_obj_set_style(btn1, &lv_style_plain_color);
  lv_obj_set_size(btn1, 120, 50);
  lv_obj_set_pos(btn1, 10, 35);
  //lv_obj_align(btn1, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  lv_obj_set_free_num(btn1, 1);   /*Set a unique number for the button*/
  lv_btn_set_action(btn1, LV_BTN_ACTION_CLICK, btn_click_action);

  /*Add a label to the button*/
  label = lv_label_create(btn1, NULL);
  lv_label_set_text(label, "Button A");

  /*Copy the button and set toggled state. (The release action is copied too)*/
  lv_obj_t * btn2 = lv_btn_create(lv_scr_act(), btn1);
  //lv_obj_set_style(btn2, &lv_style_pretty_color);
  lv_obj_align(btn2, btn1, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  //lv_btn_set_state(btn2, LV_BTN_STATE_TGL_REL);  /*Set toggled state*/
  lv_obj_set_free_num(btn2, 2);               /*Set a unique number for the button*/
  lv_btn_set_action(btn2, LV_BTN_ACTION_CLICK, btn_click_action);

  /*Add a label to the toggled button*/
  label = lv_label_create(btn2, NULL);
  lv_label_set_text(label, "Button B");

  /*Copy the button and set inactive state.*/
  lv_obj_t * btn3 = lv_btn_create(lv_scr_act(), btn2);
  //lv_obj_set_style(btn3, &style_shadow);
  lv_obj_align(btn3, btn2, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  //lv_btn_set_state(btn3, LV_BTN_STATE_TGL_REL);  /*Set toggled state*/
  //lv_btn_set_state(btn3, LV_BTN_STATE_INA);   /*Set inactive state*/
  lv_obj_set_free_num(btn3, 3);               /*Set a unique number for the button*/
  lv_btn_set_action(btn3, LV_BTN_ACTION_CLICK, btn_click_action);

  /*Add a label to the inactive button*/
  label = lv_label_create(btn3, NULL);
  //lv_label_set_text(label, "Inactive");
  lv_label_set_text(label, "Button C");

  /*Copy the button and set inactive state.*/
  lv_obj_t * btn4 = lv_btn_create(lv_scr_act(), btn3);
  //lv_obj_set_style(btn3, &style_shadow);
  lv_obj_align(btn4, btn3, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  //lv_btn_set_state(btn3, LV_BTN_STATE_TGL_REL);  /*Set toggled state*/
  //lv_btn_set_state(btn3, LV_BTN_STATE_INA);   /*Set inactive state*/
  lv_obj_set_free_num(btn4, 4);               /*Set a unique number for the button*/
  lv_btn_set_action(btn4, LV_BTN_ACTION_CLICK, btn_click_action);

  /*Add a label to the inactive button*/
  label = lv_label_create(btn4, NULL);
  //lv_label_set_text(label, "Inactive");
  lv_label_set_text(label, "Button D");

  /*Copy the button and set inactive state.*/
  lv_obj_t * btn5 = lv_btn_create(lv_scr_act(), btn4);
  //lv_obj_set_style(btn3, &style_shadow);
  lv_obj_align(btn5, btn4, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  //lv_btn_set_state(btn3, LV_BTN_STATE_TGL_REL);  /*Set toggled state*/
  //lv_btn_set_state(btn3, LV_BTN_STATE_INA);   /*Set inactive state*/
  lv_obj_set_free_num(btn5, 5);               /*Set a unique number for the button*/
  lv_btn_set_action(btn5, LV_BTN_ACTION_CLICK, btn_click_action);

  /*Add a label to the inactive button*/
  label = lv_label_create(btn5, NULL);
  //lv_label_set_text(label, "Inactive");
  lv_label_set_text(label, "Button E");

  // -----------------------------------------------------------------------
  /*Create a style for the chart*/
  static lv_style_t style;
  lv_style_copy(&style, &lv_style_pretty);
  style.body.shadow.width = 5;
  style.body.shadow.color = LV_COLOR_GRAY;
  style.line.color = LV_COLOR_GRAY;

  /*Create a chart*/
  lv_obj_t * chart;
  chart = lv_chart_create(lv_scr_act(), NULL);
  lv_obj_set_size(chart, 320, 265);
  lv_obj_set_style(chart, &style);
  //lv_obj_align(chart, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_pos(chart, 145, 39);
  lv_chart_set_type(chart, LV_CHART_TYPE_POINT | LV_CHART_TYPE_LINE);   /*Show lines and points too*/
  lv_chart_set_series_opa(chart, LV_OPA_70);                            /*Opacity of the data series*/
  lv_chart_set_series_width(chart, 4);                                  /*Line width and point radious*/

  lv_chart_set_range(chart, 0, 100);

  /*Add two data series*/
  lv_chart_series_t * ser1 = lv_chart_add_series(chart, LV_COLOR_RED);
  lv_chart_series_t * ser2 = lv_chart_add_series(chart, LV_COLOR_GREEN);
  lv_chart_series_t * ser3 = lv_chart_add_series(chart, LV_COLOR_BLUE);

  /*Set the next points on 'dl1'*/
//  lv_chart_set_next(chart, ser1, 10);
//  lv_chart_set_next(chart, ser1, 50);
//  lv_chart_set_next(chart, ser1, 70);
//  lv_chart_set_next(chart, ser1, 90);
  ser1->points[0] = 5;
  ser1->points[1] = 25;
  ser1->points[2] = 50;
  ser1->points[3] = 60;
  ser1->points[4] = 50;
  ser1->points[5] = 60;
  ser1->points[6] = 70;
  ser1->points[7] = 80;
  ser1->points[8] = 90;
  ser1->points[9] = 90;

  /*Directly set points on 'dl2'*/
  ser2->points[0] = 0;
  ser2->points[1] = 10;
  ser2->points[2] = 20;
  ser2->points[3] = 30;
  ser2->points[4] = 40;
  ser2->points[5] = 50;
  ser2->points[6] = 60;
  ser2->points[7] = 70;
  ser2->points[8] = 80;
  ser2->points[9] = 90;

  /*Directly set points on 'dl3'*/
  ser3->points[0] = 0;
  ser3->points[1] = 20;
  ser3->points[2] = 45;
  ser3->points[3] = 55;
  ser3->points[4] = 55;
  ser3->points[5] = 55;
  ser3->points[6] = 65;
  ser3->points[7] = 75;
  ser3->points[8] = 85;
  ser3->points[9] = 95;

  lv_chart_refresh(chart); /*Required after direct set*/

}


void loop() {

  lv_task_handler(); /* let the GUI do its work */
  delay(5);

}
