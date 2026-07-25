/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   battery_management.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 13:00:00 by raleksan          #+#    #+#             */
/*   Updated: 2026/07/25 18:30:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42-Smart-Cluster-Sign.h"


int16_t  read_battery_charge(void)
{
    int8_t  samples_count;
    int16_t battery;

    watchdog_reset();
    samples_count = 0;
    battery = 0;
    while (samples_count < BATTERY_SAMPLES_LIMIT)
    {
        battery += adc1_get_raw(ADC1_CHANNEL_0);
        delay(100);
        samples_count++;
    }
    battery = battery / samples_count;
    DEBUG_PRINTF("[BATTERY] Current battery state: %d\n", battery);
    return (battery);
}


/*
*   Takes appropriate action based on the battery charge.
*   Handles cases like low battery (it is time to charge),
*   critical battery (battery is too low for normal work),
*   brown-out (it is a miracle it can even boot at all).
*/
void  battery_monitor(void)
{
    int16_t battery;

    watchdog_reset();
    if (esp_reset_reason() == ESP_RST_BROWNOUT)
    {
        DEBUG_PRINTF("\n[BATTERY] Brown-out reset! Going into extensive sleep\n");
        com_g.block_validation = true;
        go_to_sleep(DEAD_BATTERY_SLEEP_MS);
    }
    battery = read_battery_charge();
    if (battery >= BATTERY_GOOD)
        return ;  
    if (battery < BATTERY_CRITICAL)
    {
        display_cluster_number(LOW_BATTERY);
        DEBUG_PRINTF("[BATTERY] Battery charge 0%! Going into extensive sleep\n\n");
        send_telegram_message(compose_message(DEAD_BATTERY, 0));
        com_g.block_validation = true;
        go_to_sleep(DEAD_BATTERY_SLEEP_MS);
    }
    else if (battery < BATTERY_GOOD)
    {
        display_cluster_number(LOW_BATTERY);
        DEBUG_PRINTF("[BATTERY] Low battery. Need charging.\n\n");
        send_telegram_message(compose_message(LOW_BATTERY, 0));
    }
}


void  battery_init(void)
{
    watchdog_reset();
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12);
}
 
