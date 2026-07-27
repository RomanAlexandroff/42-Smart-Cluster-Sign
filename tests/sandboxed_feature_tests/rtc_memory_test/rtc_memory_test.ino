/*
 *    ESP32 RTC memory retention test.
 *    raleksan, 12.2024
 */

# include "rtc_memory_test.h"

RTC_DATA_ATTR s_rtc_data rtc_g;

void setup()
{
    ft_serial_init();
    if (rtc_g.bytes_counter == 0)                       // Check if the RTC memory has already been initialized
        ft_memory_init();
    else
    {
        ft_check_memory_content();                        // Check if the memory content is intact
        ft_expand_memory_content();                       // Expand memory retention check area
    }
    Serial.print("Current bytes_counter: ");
    Serial.println(rtc_g.bytes_counter);
    esp_sleep_enable_timer_wakeup(SLEEP_TIME_S * S_TO_uS_FACTOR);
    esp_deep_sleep_start();
}

void loop()
{
    /* Nothing here. The execution loops over sleep */
}
 
