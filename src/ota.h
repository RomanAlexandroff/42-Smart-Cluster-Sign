/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ota.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/19 12:50:00 by raleksan          #+#    #+#             */
/*   Updated: 2024/12/09 17:20:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/*                                                                            */
/*   This file contains inline functions declared in the main header. This    */
/*   file has to be included AFTER all the functions' declarations.           */
/*                                                                            */
/* ************************************************************************** */

#ifndef OTA_H
# define OTA_H

void ota_init(void)
{
    if (!rtc_g.ota)
        return;
    if (WiFi.status() != WL_CONNECTED)
        wifi_connect();
    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUG_PRINTF("[OTA] Failed to initialise OTA due to Wi-Fi connection issues\n");
        rtc_g.ota = false;
        return;
    }
    watchdog_reset();
    ArduinoOTA.setHostname(DEVICE_NAME);
    ArduinoOTA
        .onStart([]() {
            String type;
            watchdog_stop();
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";
            DEBUG_PRINTF("[OTA] Start updating %s", type.c_str());
            bot.sendMessage(String(rtc_g.chat_id), "Updating...");
        })
        .onEnd([]() {
            display_cluster_number(OTA_SUCCESS);
            DEBUG_PRINTF("\n[OTA] End");
            rtc_g.ota = false;
            bot.sendMessage(String(rtc_g.chat_id), "Successfully updated!");
            delay(3000);
            ESP.restart();
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            DEBUG_PRINTF("\n[OTA] Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            watchdog_start();
            display_cluster_number(OTA_FAIL);
            DEBUG_PRINTF("\n[OTA] Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) DEBUG_PRINTF("Auth Failed\n");
            else if (error == OTA_BEGIN_ERROR) DEBUG_PRINTF("Begin Failed\n");
            else if (error == OTA_CONNECT_ERROR) DEBUG_PRINTF("Connect Failed\n");
            else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINTF("Receive Failed\n");
            else if (error == OTA_END_ERROR) DEBUG_PRINTF("End Failed\n");
            bot.sendMessage(String(rtc_g.chat_id), "Something went wrong. Updating was not completed. Try again later");
            ft_delay(3000);
            clear_display();
            ESP.restart();
        });
    display_cluster_number(OTA_WAITING);
    DEBUG_PRINTF("\n[OTA] Ready to update\n\n");
    bot.sendMessage(String(rtc_g.chat_id), "OTA Update is active");
    ArduinoOTA.begin();
}

void ota_waiting_loop(void)
{
    uint16_t ota_limit;

    if (!rtc_g.ota)
        return;
    ota_limit = 0;
    while (rtc_g.ota && ota_limit < OTA_WAIT_LIMIT_S)
    {
        ArduinoOTA.handle();
        watchdog_reset();
        DEBUG_PRINTF("\n[OTA] Active");
        ota_limit++;
        delay(1000);
    }
    rtc_g.ota = false;
    display_cluster_number(OTA_CANCELED);
    bot.sendMessage(String(rtc_g.chat_id), "OTA Update port closed");
    DEBUG_PRINTF("\n[OTA] OTA Update port closed\n");
    delay(3000);
}

#endif
 
