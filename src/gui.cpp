#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <lvgl.h>
#include "gui.h"
#include "plc_comm.h"

/* Touchscreen pins */
#define XPT2046_IRQ 36   /* T_IRQ */ 
#define XPT2046_MOSI 32  /* T_DIN */
#define XPT2046_MISO 39  /* T_OUT */
#define XPT2046_CLK 25   /* T_CLK */
#define XPT2046_CS 33    /* T_CS */

static SPIClass touchscreenSPI = SPIClass(VSPI);
static XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

static lv_obj_t * g_input_dots[6];
static lv_obj_t * g_input_texts[6];
static lv_obj_t * g_output_switches[6];

/**
 * @brief Log print callback for LVGL
 * @param[in] level Log level
 * @param[in] buf Log message
 * @return void
 */
static void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

/**
 * @brief Read touchscreen input for LVGL
 * @param[in] indev Input device
 * @param[out] data Input data
 * @return void
 */
static void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    int x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    int y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);

    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

/**
 * @brief Output switch callback for LVGL
 * @param[in] e Event data
 * @return void
 */
static void output_switch_cb(lv_event_t * e) {
    lv_obj_t * sw = (lv_obj_t *)lv_event_get_target(e);
    int index = (int)(intptr_t)lv_event_get_user_data(e);
    bool on = lv_obj_has_state(sw, LV_STATE_CHECKED);
    plc_set_output(index, on);
}

/**
 * @brief Create GUI elements for the PLC panel
 * @return void
 */
static void create_gui_elements(void) {
    int32_t screen_w = lv_obj_get_width(lv_scr_act());

    /* Header panel */
    lv_obj_t * header = lv_obj_create(lv_scr_act());
    lv_obj_set_size(header, screen_w, 40);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x0D3A6F), 0); /* Dark blue background */
    lv_obj_set_style_pad_all(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);

    /* Title on the top-left */
    lv_obj_t * title_label = lv_label_create(header);
    lv_label_set_text(title_label, "PLC PANEL");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_16, 0);
    lv_obj_align(title_label, LV_ALIGN_LEFT_MID, 12, 0);

    /* Communication status on the top-right */
    lv_obj_t * com_label = lv_label_create(header);
    lv_label_set_text(com_label, "COM:");
    lv_obj_set_style_text_color(com_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(com_label, &lv_font_montserrat_14, 0);
    lv_obj_align(com_label, LV_ALIGN_RIGHT_MID, -90, 0);

    /* Status indicator dot */
    lv_obj_t * status_dot = lv_obj_create(header);
    lv_obj_set_size(status_dot, 10, 10);
    lv_obj_set_style_radius(status_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(status_dot, lv_color_hex(0x1ACF00), 0);
    lv_obj_set_style_border_width(status_dot, 0, 0);
    lv_obj_align_to(status_dot, com_label, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    /* Status text (e.g., online / offline) */
    lv_obj_t * status_text = lv_label_create(header);
    lv_label_set_text(status_text, "online");
    lv_obj_set_style_text_color(status_text, lv_color_hex(0x1ACF00), 0);
    lv_obj_set_style_text_font(status_text, &lv_font_montserrat_14, 0);
    lv_obj_align_to(status_text, status_dot, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    /* Inputs panel (center-left between header and footer, left half) */
    lv_obj_t * inputs_panel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(inputs_panel, (screen_w / 2) - 12, 150);
    lv_obj_align(inputs_panel, LV_ALIGN_LEFT_MID, 12, 0);
    lv_obj_set_style_bg_color(inputs_panel, lv_color_hex(0xF1F4F7), 0);
    lv_obj_set_style_border_width(inputs_panel, 2, 0);
    lv_obj_set_style_border_color(inputs_panel, lv_color_hex(0x1F3A5A), 0);
    lv_obj_set_style_radius(inputs_panel, 8, 0);
    lv_obj_set_style_pad_all(inputs_panel, 4, 0); /* remove internal padding */
    lv_obj_set_layout(inputs_panel, LV_LAYOUT_FLEX); /* activate FLEX layout */
    lv_obj_set_flex_flow(inputs_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(inputs_panel, 2, 0); /* space between rows */

    const uint8_t row_count = 6; /* Number of rows */
    const int32_t row_height = 20; /* Height of each row */

    for (uint8_t input_num = 1; input_num <= row_count; input_num++) {

        char label_text[8];

        /* create row */
        lv_obj_t * row = lv_obj_create(inputs_panel);
        lv_obj_set_width(row, lv_pct(100));
        lv_obj_set_height(row, row_height);

        lv_obj_set_style_bg_color(row, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_border_color(row, lv_color_hex(0xB0B8C2), 0);
        lv_obj_set_style_radius(row, 6, 0);

        /* input label */
        snprintf(label_text, sizeof(label_text), "IN%d", input_num);

        lv_obj_t * in_label = lv_label_create(row);
        lv_label_set_text(in_label, label_text);
        lv_obj_set_style_text_color(in_label, lv_color_hex(0x1F3A5A), 0);
        lv_obj_set_style_text_font(in_label, &lv_font_montserrat_14, 0);
        lv_obj_align(in_label, LV_ALIGN_LEFT_MID, 5, 0);

        /* state indicator */
        lv_obj_t * state_dot = lv_obj_create(row);
        lv_obj_set_size(state_dot, 10, 10);
        lv_obj_set_style_radius(state_dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(state_dot, lv_color_hex(0x4CCB00), 0);
        lv_obj_set_style_border_width(state_dot, 0, 0);
        lv_obj_align(state_dot, LV_ALIGN_RIGHT_MID, -50, 0);

        g_input_dots[input_num - 1] = state_dot;

        /* state text */
        lv_obj_t * state_text = lv_label_create(row);
        lv_label_set_text(state_text, "OFF");
        lv_obj_set_style_text_color(state_text, lv_color_hex(0x1F3A5A), 0);
        lv_obj_set_style_text_font(state_text, &lv_font_montserrat_14, 0);
        lv_obj_align_to(state_text, state_dot, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

        g_input_texts[input_num - 1] = state_text;
    }

    /* Outputs panel (center-right between header and footer, right half) */
    lv_obj_t * outputs_panel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(outputs_panel, (screen_w / 2) - 12, 150);
    lv_obj_align(outputs_panel, LV_ALIGN_RIGHT_MID, -12, 0);
    lv_obj_set_style_bg_color(outputs_panel, lv_color_hex(0xF1F4F7), 0);
    lv_obj_set_style_border_width(outputs_panel, 2, 0);
    lv_obj_set_style_border_color(outputs_panel, lv_color_hex(0x1F3A5A), 0);
    lv_obj_set_style_radius(outputs_panel, 8, 0);
    lv_obj_set_style_pad_all(outputs_panel, 4, 0); /* remove internal padding */
    lv_obj_set_layout(outputs_panel, LV_LAYOUT_FLEX); /* activate FLEX layout */
    lv_obj_set_flex_flow(outputs_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(outputs_panel, 2, 0); /* space between rows */

    for (uint8_t output_num = 1; output_num <= row_count; output_num++) {

        char label_text[8];

        /* create row */
        lv_obj_t * row = lv_obj_create(outputs_panel);
        lv_obj_set_width(row, lv_pct(100));
        lv_obj_set_height(row, row_height);
        lv_obj_set_style_bg_color(row, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_border_color(row, lv_color_hex(0xB0B8C2), 0);
        lv_obj_set_style_radius(row, 6, 0);

        /* outputs states switches indicator */
        lv_obj_t * state_switch = lv_switch_create(row);
        lv_obj_align(state_switch, LV_ALIGN_RIGHT_MID, -50, 0);

        g_output_switches[output_num - 1] = state_switch;
        lv_obj_add_event_cb(state_switch, output_switch_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)(output_num - 1));

        /* output label */
        snprintf(label_text, sizeof(label_text), "OUT%d", output_num);
        lv_obj_t * out_label = lv_label_create(row);
        lv_label_set_text(out_label, label_text);
        lv_obj_set_style_text_color(out_label, lv_color_hex(0x1F3A5A), 0);
        lv_obj_set_style_text_font(out_label, &lv_font_montserrat_14, 0);
        lv_obj_align_to(out_label, state_switch, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    }

    /* Footer status bar */
    lv_obj_t * footer = lv_obj_create(lv_scr_act());
    lv_obj_set_size(footer, screen_w, 40);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(footer, lv_color_hex(0x131F34), 0);
    lv_obj_set_style_pad_all(footer, 0, 0);
    lv_obj_set_style_border_width(footer, 0, 0);

    lv_obj_t * footer_label = lv_label_create(footer);
    lv_label_set_text(footer_label, "STATUS: SYSTEM RUNNING");
    lv_obj_set_style_text_color(footer_label, lv_color_hex(0xFFD600), 0);
    lv_obj_set_style_text_font(footer_label, &lv_font_montserrat_16, 0);
    lv_obj_center(footer_label);
}

/**
 * @brief Initialize the GUI
 * @return void
 */
void gui_init(void) {
    lv_init();
    lv_log_register_print_cb(log_print);

    touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touchscreen.begin(touchscreenSPI);
    touchscreen.setRotation(2);

    lv_display_t * disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);

    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touchscreen_read);

    create_gui_elements();
}

/**
 * @brief GUI task for updating LVGL elements
 * @param[in] pvParameters FreeRTOS task parameter (unused)
 * @return void
 */
void gui_task(void *pvParameters) {
    (void)pvParameters;

    while (true) {
        /* Update input dots indicators and inputs state text*/ 
        for (int i = 0; i < 6; i++) {
            bool in_state = plc_get_input(i);
            lv_obj_set_style_bg_color(g_input_dots[i], in_state ? lv_color_hex(0x4CCB00) : lv_color_hex(0x6B6F7B), 0);
            lv_label_set_text(g_input_texts[i], in_state ? "ON" : "OFF");
        }

        /* Update output switches (in case they change externally) */
        for (int i = 0; i < 6; i++) {
            bool out_state = plc_get_output(i);
            if (out_state) {
                lv_obj_add_state(g_output_switches[i], LV_STATE_CHECKED);
            } else {
                lv_obj_clear_state(g_output_switches[i], LV_STATE_CHECKED);
            }
        }

        lv_task_handler();
        lv_tick_inc(5);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
