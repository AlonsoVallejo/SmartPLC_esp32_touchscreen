#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <lvgl.h>

/* Touchscreen pins */
#define XPT2046_IRQ 36   /* T_IRQ */ 
#define XPT2046_MOSI 32  /* T_DIN */
#define XPT2046_MISO 39  /* T_OUT */
#define XPT2046_CLK 25   /* T_CLK */
#define XPT2046_CS 33    /* T_CS */

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

/* Touchscreen coordinates: (x, y) and pressure (z) */
int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

/* If logging is enabled, it will inform the user about what is happening in the library */ 
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

/* Get the Touchscreen data */ 
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  /* Checks if Touchscreen was touched, and prints X, Y and Pressure (Z) */
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    /* Get Touchscreen points */
    TS_Point p = touchscreen.getPoint();
    /* Calibrate Touchscreen points with map function to the correct width and height */
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    /* Set the coordinates */
    data->point.x = x;
    data->point.y = y;
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

static void switch_event_cb(lv_event_t * e) {
    lv_obj_t * sw = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(e);
    if(lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        lv_label_set_text(label, "ON");
    } else {
        lv_label_set_text(label, "OFF");
    }
}

void lv_create_main_gui(void) {
    /* Header panel (matches the sample image style) */
    lv_obj_t * header = lv_obj_create(lv_screen_active());
    int32_t screen_w = lv_obj_get_width(lv_scr_act());
    lv_obj_set_size(header, screen_w, 40);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x0D3A6F), 0); // Dark blue background
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
    lv_obj_set_style_bg_color(status_dot, lv_color_hex(0x1ACF00), 0); // Green
    lv_obj_set_style_border_width(status_dot, 0, 0);
    lv_obj_align_to(status_dot, com_label, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    /* Status text (e.g., online / offline) */
    lv_obj_t * status_text = lv_label_create(header);
    lv_label_set_text(status_text, "online");
    lv_obj_set_style_text_color(status_text, lv_color_hex(0x1ACF00), 0);
    lv_obj_set_style_text_font(status_text, &lv_font_montserrat_14, 0);
    lv_obj_align_to(status_text, status_dot, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    /* Inputs panel (center-left between header and footer, left half) */
    lv_obj_t * inputs_panel = lv_obj_create(lv_screen_active());
    lv_obj_set_size(inputs_panel, (screen_w / 2) - 12, 150);
    lv_obj_align(inputs_panel, LV_ALIGN_LEFT_MID, 12, 0);
    lv_obj_set_style_bg_color(inputs_panel, lv_color_hex(0xF1F4F7), 0);
    lv_obj_set_style_border_width(inputs_panel, 2, 0);
    lv_obj_set_style_border_color(inputs_panel, lv_color_hex(0x1F3A5A), 0);
    lv_obj_set_style_radius(inputs_panel, 8, 0);

    const uint8_t row_count = 6; /* Number of rows */
    const int32_t row_height = 20; /* Height of each row */
    const int32_t  row_spacing = 2; /* Spacing between rows */
    int32_t y_offset = -7; /* Offset from the top of the panel */
    int32_t x_offset = -7; /* Offset from the left of the panel */

    for(int i = 0; i <= row_count; i++) {
        char label_text[8];
        snprintf(label_text, sizeof(label_text), "IN%d", i);

        lv_obj_t * row = lv_obj_create(inputs_panel);
        lv_obj_set_size(row, lv_obj_get_width(inputs_panel) - 20, row_height);
        lv_obj_align(row, LV_ALIGN_TOP_LEFT, x_offset, y_offset + (i-1) * (row_height + row_spacing));
        lv_obj_set_style_bg_color(row, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_border_color(row, lv_color_hex(0xB0B8C2), 0);
        lv_obj_set_style_radius(row, 6, 0);

        lv_obj_t * in_label = lv_label_create(row);
        lv_label_set_text(in_label, label_text);
        lv_obj_set_style_text_color(in_label, lv_color_hex(0x1F3A5A), 0);
        lv_obj_set_style_text_font(in_label, &lv_font_montserrat_14, 0);
        lv_obj_align(in_label, LV_ALIGN_LEFT_MID, 1, 0);

        // Status dot + text
        lv_obj_t * state_dot = lv_obj_create(row);
        lv_obj_set_size(state_dot, 10, 10);
        lv_obj_set_style_radius(state_dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(state_dot, lv_color_hex(0x4CCB00), 0); // Green for ON
        lv_obj_set_style_border_width(state_dot, 0, 0);
        lv_obj_align(state_dot, LV_ALIGN_RIGHT_MID, -50, 0);

        lv_obj_t * state_text = lv_label_create(row);
        lv_label_set_text(state_text, "ON");
        lv_obj_set_style_text_color(state_text, lv_color_hex(0x1F3A5A), 0);
        lv_obj_set_style_text_font(state_text, &lv_font_montserrat_14, 0);
        lv_obj_align_to(state_text, state_dot, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    } 

    /* Output panel (center-right between header and footer, right half) */
    lv_obj_t * output_panel = lv_obj_create(lv_screen_active());
    lv_obj_set_size(output_panel, (screen_w / 2) - 12, 150);
    lv_obj_align(output_panel, LV_ALIGN_RIGHT_MID, -12, 0);
    lv_obj_set_style_bg_color(output_panel, lv_color_hex(0xF1F4F7), 0);
    lv_obj_set_style_border_width(output_panel, 2, 0);
    lv_obj_set_style_border_color(output_panel, lv_color_hex(0x1F3A5A), 0);
    lv_obj_set_style_radius(output_panel, 8, 0);

    for(int i = 0; i <= row_count; i++) {
        char label_text[8];
        snprintf(label_text, sizeof(label_text), "OUT%d", i);

        lv_obj_t * row = lv_obj_create(output_panel);
        lv_obj_set_size(row, lv_obj_get_width(output_panel) - 20, row_height);
        lv_obj_align(row, LV_ALIGN_TOP_LEFT, x_offset, y_offset + (i-1) * (row_height + row_spacing));
        lv_obj_set_style_bg_color(row, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_border_color(row, lv_color_hex(0xB0B8C2), 0);
        lv_obj_set_style_radius(row, 6, 0);

        lv_obj_t * out_label = lv_label_create(row);
        lv_label_set_text(out_label, label_text);
        lv_obj_set_style_text_color(out_label, lv_color_hex(0x1F3A5A), 0);
        lv_obj_set_style_text_font(out_label, &lv_font_montserrat_14, 0);
        lv_obj_align(out_label, LV_ALIGN_LEFT_MID, 1, 0);

        /* outputs states switches indicator */
        lv_obj_t * state_switch = lv_switch_create(row);
        lv_obj_align(state_switch, LV_ALIGN_RIGHT_MID, -50, 0);

        lv_obj_t * state_text = lv_label_create(row);
        lv_label_set_text(state_text, "OFF");
        lv_obj_set_style_text_color(state_text, lv_color_hex(0x1F3A5A), 0);
        lv_obj_set_style_text_font(state_text, &lv_font_montserrat_14, 0);
        lv_obj_align_to(state_text, state_switch, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

        lv_obj_add_event_cb(state_switch, switch_event_cb, LV_EVENT_VALUE_CHANGED, state_text);
    }

    /* Footer status bar (matches header height and spans full width) */
    lv_obj_t * footer = lv_obj_create(lv_screen_active());
    lv_obj_set_size(footer, screen_w, 40);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(footer, lv_color_hex(0x131F34), 0); // Darker blue/gray background
    lv_obj_set_style_pad_all(footer, 0, 0);
    lv_obj_set_style_border_width(footer, 0, 0);

    lv_obj_t * footer_label = lv_label_create(footer);
    lv_label_set_text(footer_label, "STATUS: SYSTEM RUNNING");
    lv_obj_set_style_text_color(footer_label, lv_color_hex(0xFFD600), 0); // Yellow text
    lv_obj_set_style_text_font(footer_label, &lv_font_montserrat_16, 0);
    lv_obj_center(footer_label);
}

void setup() {
    String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial.begin(9600);
    Serial.println(LVGL_Arduino);
    
    /* Start LVGL */
    lv_init();
    /* Register print function for debugging */
    lv_log_register_print_cb(log_print);

    /* Start the SPI for the touchscreen and init the touchscreen */
    touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touchscreen.begin(touchscreenSPI);
    /* Set the Touchscreen rotation in landscape mode */
    /* Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 0: touchscreen.setRotation(0); */ 
    touchscreen.setRotation(2);

    /* Create a display object */
    lv_display_t * disp;
    /* Initialize the TFT display using the TFT_eSPI library */
    disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
        
    /* Initialize an LVGL input device object (Touchscreen) */ 
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    /* Set the callback function to read Touchscreen input */ 
    lv_indev_set_read_cb(indev, touchscreen_read);

    // Function to draw the GUI (text, buttons and sliders)
    lv_create_main_gui();
}

void loop() {
    lv_task_handler();  // let the GUI do its work
    lv_tick_inc(5);     // tell LVGL how much time has passed
    delay(5);           // let this time pass
}