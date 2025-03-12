#include "ST7789.h"
#include "SD_SPI.h"
#include "RGB.h"
#include "Wireless.h"

void app_main(void)
{
    Wireless_Init();
    Flash_Searching();
    RGB_Init();
    RGB_Example();
    SD_Init();
    LCD_Init();
    BK_Light(50);
    LVGL_Init();

    while (1) {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }
}
