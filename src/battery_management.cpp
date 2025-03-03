/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   battery_management.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 13:00:00 by raleksan          #+#    #+#             */
/*   Updated: 2024/12/09 17:20:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42-Smart-Cluster-Sign.h"

void  battery_check(void)
{
    int8_t  samples_count;
    int16_t battery;

    samples_count = 0;
    battery = 0;
    watchdog_reset();
    while (samples_count < BATTERY_SAMPLES_LIMIT)
    {
        battery += adc1_get_raw(ADC1_CHANNEL_0);
        delay(100);
        samples_count++;
    }
    battery = battery / samples_count;
    DEBUG_PRINTF("[BATTERY] Current battery state: %d\n\n", battery);
    if (battery >= BATTERY_GOOD)
        return;
    if (WiFi.status() != WL_CONNECTED)
        wifi_connect();    
    if (battery < BATTERY_CRITICAL)
    {
        display_cluster_number(LOW_BATTERY);
        DEBUG_PRINTF("[BATTERY] Battery is too low. Going into extensive sleep\n\n");
        bot.sendMessage(String(rtc_g.chat_id), compose_message(DEAD_BATTERY, 0), "");
        go_to_sleep(DEAD_BATTERY_SLEEP_MS);
    }
    else if (battery < BATTERY_GOOD)
    {
        display_cluster_number(LOW_BATTERY);
        DEBUG_PRINTF("[BATTERY] Low battery! Need charging!\n\n");
        bot.sendMessage(String(rtc_g.chat_id), compose_message(LOW_BATTERY, 0), "");
    }
}

void  battery_init(void)
{
    watchdog_reset();
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12);
}
 
