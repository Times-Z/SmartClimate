#include "global.h"
#include <esp_random.h>

static const char *TAG = "UI:GLOBAL";
extern char *APP_NAME;
extern char *APP_VERSION;
static lv_obj_t *main_ui_container = NULL;

#define CAT_FRAMES 3

static lv_obj_t *cat_label;
static lv_timer_t *cat_timer;
static int cat_frame_index = 0;

static const char *cat_frames[CAT_FRAMES] = {
    " |\\___/| \n"
    " )     (  \n"
    "=\\     /=\n"
    "  )===(   \n"
    " /     \\ \n"
    " |      |  \n"
    "/       \\\n"
    "\\       /\n"
    " \\__  _/ \n"
    "   ( (    \n"
    "    ) )   \n"
    "   (_(    ",

    " |\\___/| \n"
    " )     (  \n"
    "=\\     /=\n"
    "  )===(   \n"
    " /     \\ \n"
    " |      |  \n"
    "/       \\\n"
    "\\       /\n"
    " \\__  _/ \n"
    "  ( (     \n"
    "   ) )    \n"
    "   (_(    ",

    " |\\___/| \n"
    " )     (  \n"
    "=\\     /=\n"
    "  )===(   \n"
    " /     \\ \n"
    " |      |  \n"
    "/       \\\n"
    "\\       /\n"
    " \\__  _/ \n"
    "     ( (  \n"
    "     ) )  \n"
    "    (_(    "};

static void cat_idle_anim_cb(lv_timer_t *timer) {
    cat_frame_index = (cat_frame_index + 1) % CAT_FRAMES;
    lv_label_set_text(cat_label, cat_frames[cat_frame_index]);

    uint32_t random_delay = 300 + (esp_random() % 900);
    lv_timer_set_period(cat_timer, random_delay);
}

void ui_clear_screen(void) {
    lv_obj_t *screen = lv_scr_act();
    lv_obj_clean(screen);
}

void ui_clear_main_container(void) {
    lv_obj_t *container = ui_get_main_container();
    if (container != NULL) {
        lv_obj_clean(container);
    }
}

void ui_show_idle_cat_animation(void) {
    lv_obj_t *container = ui_get_main_container();

    cat_label = lv_label_create(container);
    lv_label_set_text(cat_label, cat_frames[0]);
    lv_label_set_long_mode(cat_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(cat_label, lv_disp_get_hor_res(NULL));
    lv_obj_set_style_text_align(cat_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(cat_label, 0, 0);
    lv_obj_set_style_pad_all(cat_label, 0, 0);
    lv_obj_set_style_text_font(cat_label, &lv_font_unscii_8, 0);

    cat_timer = lv_timer_create(cat_idle_anim_cb, 400, NULL);
}

/// @brief Get or create the main UI container
/// @param void
/// @return lv_obj_t* Pointer to the initialized main UI container object.
lv_obj_t *ui_get_main_container(void) {
    if (main_ui_container == NULL) {
        ESP_LOGI(TAG, "Create main container");
        lv_obj_t *screen = lv_screen_active();
        static lv_style_t child_label_style;

        main_ui_container = lv_obj_create(screen);

        lv_obj_set_size(main_ui_container, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
        lv_obj_align(main_ui_container, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_flex_flow(main_ui_container, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(main_ui_container,
                              LV_FLEX_ALIGN_CENTER,  // horizontal
                              LV_FLEX_ALIGN_CENTER,  // vertical
                              LV_FLEX_ALIGN_CENTER   // cross-axis
        );
        lv_obj_set_style_bg_opa(main_ui_container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(main_ui_container, 0, 0);

        lv_style_init(&child_label_style);
        lv_style_set_text_align(&child_label_style, LV_TEXT_ALIGN_CENTER);
        lv_style_set_text_color(&child_label_style, lv_color_black());

        lv_obj_add_style(main_ui_container, &child_label_style, 0);
        lv_obj_set_scroll_dir(main_ui_container, LV_DIR_NONE);
    }
    return main_ui_container;
}

/// @brief Show labels with app name and version
/// @param void
/// @return void
void ui_show_app_name_and_version(void) {
    ESP_LOGI(TAG, "Displaying app name & version...");

    lv_obj_t *app_label = NULL;

    if (app_label == NULL) {
        lv_obj_t *container = ui_get_main_container();
        app_label = lv_label_create(container);
        lv_obj_set_style_text_color(app_label, lv_color_black(), 0);
        lv_obj_set_width(app_label, lv_disp_get_hor_res(NULL));
        lv_label_set_long_mode(app_label, LV_LABEL_LONG_WRAP);
    }

    lv_label_set_text_fmt(app_label, "%s\nversion %s", APP_NAME, APP_VERSION);
}

/// @brief show a centerd spinner (for loading)
/// @param void
// @return void
void ui_show_loader_spinner(void) {
    lv_obj_t *container = ui_get_main_container();
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *spinner = lv_spinner_create(container);
    lv_obj_set_size(spinner, UI_MAX_SIZE * 0.5, UI_MAX_SIZE * 0.5);

    lv_obj_set_style_arc_color(spinner, lv_color_black(), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, UI_MAX_SIZE * 0.05, LV_PART_INDICATOR);

    lv_obj_set_style_arc_opa(spinner, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(spinner, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(spinner, LV_OPA_TRANSP, LV_PART_MAIN);
}

/// @brief show the desired message
/// @param char message_string
/// @return void
void ui_show_message(char *message_string) {
    ESP_LOGI(TAG, "Displaying %s", message_string);

    lv_obj_t *label = NULL;
    lv_obj_t *container = ui_get_main_container();
    label = lv_label_create(container);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
    lv_obj_set_width(label, lv_disp_get_hor_res(NULL));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    lv_label_set_text_fmt(label, "%s", message_string);
}