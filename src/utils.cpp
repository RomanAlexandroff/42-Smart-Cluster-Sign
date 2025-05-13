/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 13:10:00 by raleksan          #+#    #+#             */
/*   Updated: 2024/12/18 13:00:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42-Smart-Cluster-Sign.h"

void  go_to_sleep(uint64_t time_in_millis)
{
    esp_err_t result;

    watchdog_stop();
    if (time_in_millis < MIN_SLEEP_LIMIT_MS)
        time_in_millis = MIN_SLEEP_LIMIT_MS;
    if (time_in_millis > MAX_SLEEP_LIMIT_MS)
        time_in_millis = MAX_SLEEP_LIMIT_MS;
    display.powerOff();
    result = esp_deep_sleep_enable_gpio_wakeup(GPIO_MASK, ESP_GPIO_WAKEUP_GPIO_LOW);
    if (result != ESP_OK)
        DEBUG_PRINTF("\nFailed to set up wake-up with a button.\n\n");
    DEBUG_PRINTF("The device was running for %lu second(s) this time\n", (millis() / mS_TO_S_FACTOR));
    DEBUG_PRINTF("Going to sleep for %llu seconds.\n", time_in_millis / mS_TO_S_FACTOR);
    DEBUG_PRINTF("\nDEVICE STOP\n\n\n");
    esp_sleep_enable_timer_wakeup(time_in_millis * mS_TO_uS_FACTOR);
    esp_deep_sleep_start();
}

/*
*   Delays and puts to sleep Wi-Fi and
*   Bluetooth modems to preserve
*   the battery charge
*/
void  ft_delay(uint64_t time_in_millis)
{
    if (time_in_millis <= 1)
    {
        delay(time_in_millis);
        return;
    }
    watchdog_stop();
    if (time_in_millis > MAX_SLEEP_LIMIT_MS)
        time_in_millis = MAX_SLEEP_LIMIT_MS;
    esp_sleep_enable_timer_wakeup(time_in_millis * mS_TO_uS_FACTOR);
    if (esp_light_sleep_start() != ESP_OK)
    {
        DEBUG_PRINTF("\n[FT_DELAY] Light sleep fail. Delaying without sleep.\n");   // 52 characters
        #ifdef DEBUG
            uint64_t debug_print_time_ms = ceil(52.0 * 10 / BAUD_RATE * 1000);
            if (time_in_millis > debug_print_time_ms)
                time_in_millis -= debug_print_time_ms;
            else
                time_in_millis = 0;
        #endif
        delay(time_in_millis);
    }
    watchdog_start();
}

void  wifi_connect(void)
{
    short i;

    i = 0;
    watchdog_reset();
    WiFi.mode(WIFI_STA);
    WiFi.persistent(true);
    Telegram_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while ((WiFi.status() != WL_CONNECTED) && i < CONNECT_TIMEOUT_S)
    {
        watchdog_reset();
        delay(1000);
        i++;
    }
}

void  serial_init(void)
{
    uint8_t i;
    
    i = 15;
    watchdog_reset();
    Serial.begin(BAUD_RATE);
    while (i > 0)
    {
        DEBUG_PRINTF("-");
        delay(100);
        i--;
    }
    DEBUG_PRINTF("\n\nDEVICE START\nversion %.2f\n", SOFTWARE_VERSION);
}

#ifdef EXAM_SIMULATION
    String  exam_simulation(void)
    {
        String  day;
        String  month;
        String  virtual_exam;

        watchdog_reset();
        if (com_g.month < 10)
            month = "0" + String(com_g.month);
        else
            month = String(com_g.month);
        if (com_g.day < 10)
            day = "0" + String(com_g.day);
        else
            day = String(com_g.day);
        virtual_exam = "[{\"begin_at\":\"";
        virtual_exam += String(com_g.year) + "-" + month + "-" + day + "T20:00:00.000Z\",";           // Change begin time here. Mind the TIME_ZONE correction
        virtual_exam += "\"end_at\":\"";
        virtual_exam += String(com_g.year) + "-" + month + "-" + day + "T22:00:00.000Z\",";           // Change end time here. Mind the TIME_ZONE correction
        virtual_exam += "\"nbr_subscribers\":4,}]";
        DEBUG_PRINTF("\n\n[EXAM SIMULATION] ATTENTION! EXAM SIMULATION IS ACTIVE!\n");
        DEBUG_PRINTF("[EXAM SIMULATION] The following exam is just a simulation!\n");
        DEBUG_PRINTF("[EXAM SIMULATION] Virtual exam data as follows:\n%s\n\n", virtual_exam.c_str());
        return (virtual_exam);
    }
#endif
 
