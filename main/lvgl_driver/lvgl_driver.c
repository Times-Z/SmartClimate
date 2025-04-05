
#include "lvgl_driver.h"

#define LCD_H_RES 172
#define LCD_V_RES 320
#define LVGL_TICK_PERIOD_MS 2
#define Offset_X 0
#define Offset_Y 0
#define LV_BUFFER_LINES 40

static const char *TAG = "LVGL";
static lv_fs_drv_t fs_drv;

lv_display_t *disp;

// custom lvgl memory management
void *lv_malloc_core(size_t size) { return heap_caps_malloc(size, MALLOC_CAP_DMA); }

void lv_free_core(void *ptr) { heap_caps_free(ptr); }

void *lv_realloc_core(void *ptr, size_t new_size) {
    if (!ptr) return lv_malloc_core(new_size);
    void *new_ptr = lv_malloc_core(new_size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, new_size);
        lv_free_core(ptr);
    }
    return new_ptr;
}
void lv_mem_init(void) {}

void lvgl_increase_lvgl_tick(void *arg) { lv_tick_inc(LVGL_TICK_PERIOD_MS); }

void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1 + Offset_X, area->y1 + Offset_Y, area->x2 + Offset_X + 1,
                              area->y2 + Offset_Y + 1, px_map);
    lv_display_flush_ready(disp);
}

void lvgl_port_update_callback(lv_display_t *disp) {
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);
    switch (lv_display_get_rotation(disp)) {
        case LV_DISPLAY_ROTATION_0:
            esp_lcd_panel_swap_xy(panel_handle, false);
            esp_lcd_panel_mirror(panel_handle, true, false);
            break;
        case LV_DISPLAY_ROTATION_90:
            esp_lcd_panel_swap_xy(panel_handle, true);
            esp_lcd_panel_mirror(panel_handle, true, true);
            break;
        case LV_DISPLAY_ROTATION_180:
            esp_lcd_panel_swap_xy(panel_handle, false);
            esp_lcd_panel_mirror(panel_handle, false, true);
            break;
        case LV_DISPLAY_ROTATION_270:
            esp_lcd_panel_swap_xy(panel_handle, true);
            esp_lcd_panel_mirror(panel_handle, false, false);
            break;
    }
}

static void *lvgl_fs_open_cb(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode) {
    LV_UNUSED(drv);

    char full_path[256];
    snprintf(full_path, sizeof(full_path), "/sdcard%s", path);

    const char *flags = (mode == LV_FS_MODE_WR || mode == (LV_FS_MODE_WR | LV_FS_MODE_RD)) ? "rb+"
                        : (mode == LV_FS_MODE_RD)                                          ? "rb"
                                                                                           : "rb";

    FILE *f = fopen(full_path, flags);

    if (!f) {
        ESP_LOGW(TAG, "Failed to open: %s", full_path);
    } else {
        ESP_LOGI(TAG, "Opened file: %s", full_path);
    }

    return f;
}

static lv_fs_res_t lvgl_fs_close_cb(lv_fs_drv_t *drv, void *file_p) {
    LV_UNUSED(drv);
    fclose((FILE *)file_p);
    return LV_FS_RES_OK;
}

static lv_fs_res_t lvgl_fs_read_cb(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br) {
    LV_UNUSED(drv);
    *br = fread(buf, 1, btr, (FILE *)file_p);
    return LV_FS_RES_OK;
}

static lv_fs_res_t lvgl_fs_seek_cb(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence) {
    LV_UNUSED(drv);

    int origin = (whence == LV_FS_SEEK_SET)   ? SEEK_SET
                 : (whence == LV_FS_SEEK_CUR) ? SEEK_CUR
                 : (whence == LV_FS_SEEK_END) ? SEEK_END
                                              : SEEK_SET;

    fseek((FILE *)file_p, pos, origin);
    return LV_FS_RES_OK;
}

static lv_fs_res_t lvgl_fs_tell_cb(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p) {
    LV_UNUSED(drv);
    *pos_p = ftell((FILE *)file_p);
    return LV_FS_RES_OK;
}

/// @brief register a virtual fs to sd card
/// @param void
/// @return void
void lvgl_fs_register_sdcard(void) {
    lv_fs_drv_init(&fs_drv);
    fs_drv.letter = 'S';
    fs_drv.open_cb = lvgl_fs_open_cb;
    fs_drv.close_cb = lvgl_fs_close_cb;
    fs_drv.read_cb = lvgl_fs_read_cb;
    fs_drv.seek_cb = lvgl_fs_seek_cb;
    fs_drv.tell_cb = lvgl_fs_tell_cb;

    lv_fs_drv_register(&fs_drv);

    ESP_LOGI(TAG, "LVGL FS registered as S:/ mapped to /sdcard");
}

/// @brief Init lvgl lib
/// @param void
/// @return void
void lvgl_init(void) {
    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    disp = lv_display_create(LCD_H_RES, LCD_V_RES);
    lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_color_t *buf1 = heap_caps_malloc(sizeof(lv_color_t) * LCD_H_RES * LV_BUFFER_LINES, MALLOC_CAP_DMA);
    lv_color_t *buf2 = heap_caps_malloc(sizeof(lv_color_t) * LCD_H_RES * LV_BUFFER_LINES, MALLOC_CAP_DMA);

    assert(buf1 && buf2 && "Failed to allocate LVGL draw buffers");

    lv_draw_buf_t *draw_buf1 = malloc(sizeof(lv_draw_buf_t));
    lv_draw_buf_t *draw_buf2 = malloc(sizeof(lv_draw_buf_t));
    assert(draw_buf1 && draw_buf2);

    uint32_t stride = LCD_H_RES * sizeof(lv_color_t);

    lv_draw_buf_init(draw_buf1, LCD_H_RES, 40, LV_COLOR_FORMAT_NATIVE, stride, buf1,
                     LCD_H_RES * 40 * sizeof(lv_color_t));
    lv_draw_buf_init(draw_buf2, LCD_H_RES, 40, LV_COLOR_FORMAT_NATIVE, stride, buf2,
                     LCD_H_RES * 40 * sizeof(lv_color_t));

    lv_display_set_draw_buffers(disp, draw_buf1, draw_buf2);

    lv_display_set_flush_cb(disp, lvgl_flush_cb);

    extern esp_lcd_panel_handle_t panel_handle;
    lv_display_set_user_data(disp, panel_handle);

    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_0);
    lvgl_port_update_callback(disp);

    lvgl_fs_register_sdcard();

    ESP_LOGI(TAG, "Install LVGL tick timer");
    const esp_timer_create_args_t lvgl_tick_timer_args = {.callback = &lvgl_increase_lvgl_tick, .name = "lvgl_tick"};
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));
}
