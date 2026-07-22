/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 13:10:00 by raleksan          #+#    #+#             */
/*   Updated: 2026/07/18 14:00:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42-Smart-Cluster-Sign.h"


/*
 *   This is always the last function to be executed in any scenario.
 *   Prepares the device for Deep Sleep by powering off the display,
 *   configuring timer and GPIO wake-up sources, entering Deep Sleep.
 *   This function marks the currently running firmware as VERIFIED
 *   for the firmware update rollback feature. Caller is responsible
 *   to make sure the value to be passed to this function cannot be
 *   negative. The sleep duration is clamped to the supported range.
 *   This function never returns.
*/
void  go_to_sleep(uint64_t time_in_millis)
{
    esp_err_t result;

    watchdog_stop();
    if (time_in_millis < MIN_SLEEP_LIMIT_MS)
        time_in_millis = MIN_SLEEP_LIMIT_MS;
    if (time_in_millis > MAX_SLEEP_LIMIT_MS)
        time_in_millis = MAX_SLEEP_LIMIT_MS;
    display.powerOff();
    set_rollback_flag(VERIFIED);
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
*   Low-power equivalent of delay(). The caller
*   is responsible to make sure the value to be
*   passed to this function cannot be negative.
*   Puts the CPU and the Wi-Fi modem to sleep
*   for 'time_in_millis' to preserve the battery
*   charge. RAM values stay intact and execution
*   can continue from where it stopped.
*   If the device looses Wi-Fi connection or not
*   depends on the length of 'time_in_millis' and
*   on the Wi-Fi Access Point settings - subject
*   of individual research.
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


/*
*   Wrapper for wifi_connect(). Ensures that the device is connected
*   to Wi-Fi. Returns immediately if a Wi-Fi connection is already
*   established. Otherwise, attempts to connect to the configured
*   network and reports a failure if cannot connect.
*/
bool ensure_wifi_connection(void)
{
    if (WiFi.status() == WL_CONNECTED)
        return (true);
    wifi_connect();
    watchdog_reset();
    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUG_PRINTF("\n[Wi-Fi] Unable to connect to Wi-Fi\n\n");
        return (false);
    }
    return (true);
}


/*
*   Connects the device to the configured Wi-Fi network.
*   On the first call, it initializes the Wi-Fi subsystem, including
*   enabling Modem Sleep, configuring persistent Wi-Fi credentials,
*   and loading the Telegram TLS root certificate. Waits until
*   a connection is established or the connection timeout expires.
*/
void  wifi_connect(void)
{
    static bool used = false;
    short       i;

    i = 0;
    watchdog_reset();
    WiFi.mode(WIFI_STA);
    if (!used)
    {
        if (esp_wifi_set_ps(WIFI_PS_MIN_MODEM) != ESP_OK)
            DEBUG_PRINTF("\n[Wi-Fi] Failed to enable Wi-Fi Modem Sleep\n");
        Telegram_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
        used = true;
    }
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


/*
*   Ensures the Bluetooth modem is OFF
*/
void  bluetooth_deinit(void)
{
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
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
 
