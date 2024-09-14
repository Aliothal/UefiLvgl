#include "../lvgl.h"

typedef struct {
  lv_obj_t *win_main;
  lv_obj_t *header;
  lv_obj_t *usage_cpu;
  lv_obj_t *time;
  lv_timer_t *usage_timer;
  lv_obj_t *btn_cls;
  lv_obj_t *content;
  lv_obj_t *lb_author;
  lv_style_t btn_cls_clicked;

  lv_obj_t *hex_view;
  lv_obj_t *ta_debug;
} lv_ui;

lv_ui ui;
static char debug_buf[2048];

static void lv_debug(const char* format, ...){
  VA_LIST va;
  VA_START(va, format);
  AsciiVSPrint(debug_buf, 2048, format, va);
  VA_END(va);
  lv_textarea_add_text(ui.ta_debug, debug_buf);
}

static void event_close(lv_event_t * e)
{
  lv_point_t point;
  lv_indev_get_point(lv_event_get_indev(e), &point);
  if (lv_obj_hit_test(lv_event_get_target_obj(e), &point)) {
    lv_timer_delete(ui.usage_timer);
    lv_obj_delete(ui.win_main);
    lv_efi_app_exit();
  }
}

static void show_usage(lv_timer_t * t)
{
  EFI_TIME time;
  gRT->GetTime(&time, NULL);
  lv_label_set_text_fmt(ui.usage_cpu, "Lvgl cpu usage: %3d%%", (100 - lv_timer_get_idle()));
  lv_label_set_text_fmt(ui.time, "Time: %4d.%2d.%2d:%2d.%2d.%2d", time.Year, time.Month, time.Day, time.Hour, time.Minute, time.Second);
}

static void event_hex(lv_event_t * e)
{
  lv_point_t point;
  if (e->code == LV_EVENT_CLICKED)
  lv_indev_get_point(lv_event_get_indev(e), &point);
}

void lv_efi_app_main(void)
{
  ui.win_main = lv_win_create(lv_screen_active());
  lv_win_add_title(ui.win_main, "HexEdit");
  ui.header = lv_win_get_header(ui.win_main);
  ui.usage_cpu = lv_label_create(ui.header);
  ui.time = lv_label_create(ui.header);
  
  ui.usage_timer = lv_timer_create(show_usage, 500, NULL);
  lv_timer_ready(ui.usage_timer);
  ui.btn_cls = lv_win_add_button(ui.win_main, LV_SYMBOL_CLOSE, 45);
  lv_style_init(&ui.btn_cls_clicked);
  lv_style_set_bg_color(&ui.btn_cls_clicked, lv_color_make(241, 112, 122));
  lv_obj_add_style(ui.btn_cls, &ui.btn_cls_clicked, LV_STATE_PRESSED);
  lv_obj_remove_state(ui.btn_cls, LV_STATE_FOCUSED);
  lv_obj_add_event_cb(ui.btn_cls, event_close, LV_EVENT_RELEASED, NULL);
  ui.content = lv_win_get_content(ui.win_main);

  ui.hex_view = lv_table_create(ui.content);
  // lv_obj_set_align(ui.hex_view, LV_ALIGN_CENTER);
  UINT8 row = 0x11, col = 0x11;
  UINT8 *data = lv_malloc_zeroed(row * col);
  lv_table_set_row_count(ui.hex_view, row);
  lv_table_set_column_count(ui.hex_view, col);
  for (UINT8 i = 0; i < col; i++) {
    lv_table_set_column_width(ui.hex_view, i, 32);
  }
  for (UINT8 i = 0; i < row; i++) {
    for (UINT8 j = 0; j < col; j++) {
      if (i == 0 && j != 0) {
        lv_table_set_cell_value_fmt(ui.hex_view, i, j, "%02X", j-1);
      } else if (i != 0 && j == 0) {
        lv_table_set_cell_value_fmt(ui.hex_view, i, j, "%02X", (i-1) << 4);
      } else {
        lv_table_set_cell_value_fmt(ui.hex_view, i, j, "%02x", data[i*col+j]);
      }
    }
  }
  lv_obj_add_event_cb(ui.hex_view, event_hex, LV_EVENT_ALL, NULL);
  lv_obj_set_style_border_width(ui.hex_view, 1, 0);
  lv_obj_set_style_border_color(ui.hex_view, lv_color_make(120,120,120), 0);

  ui.ta_debug = lv_textarea_create(ui.content);
  lv_obj_align_to(ui.ta_debug, ui.hex_view, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
  lv_obj_set_size(ui.ta_debug, 400, 800);
  lv_debug("w:%d h:%d\n", lv_obj_get_width(ui.hex_view), lv_obj_get_height(ui.hex_view));
}