/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   watchdog.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/27 15:50:00 by raleksan          #+#    #+#             */
/*   Updated: 2024/12/09 17:20:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42-Smart-Cluster-Sign.h"

void  watchdog_start(void)
{
    if (esp_task_wdt_status(NULL) != ESP_OK)
        esp_task_wdt_add(NULL);
    else
        DEBUG_PRINTF("[WATCHDOG] Watchdog was not started because it has been already running\n\n");
    esp_task_wdt_reset();
}

void  watchdog_reset(void)
{
    if (esp_task_wdt_reset() != ESP_OK)
        DEBUG_PRINTF("[WATCHDOG] Watchdog was not reset\n\n");
}

void  watchdog_stop(void)
{
    if (esp_task_wdt_status(NULL) == ESP_OK)
        esp_task_wdt_delete(NULL);
    else
        DEBUG_PRINTF("[WATCHDOG] Watchdog was not stopped because it has not been running\n\n");
}

void  watchdog_init(void)
{
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = WD_TIMEOUT_MS,
        .idle_core_mask = (1 << WD_NUMBER_OF_CORES) - 1,
        .trigger_panic = WD_RESET_INFO,
    };
    esp_task_wdt_deinit();
    esp_task_wdt_init(&twdt_config);
    esp_task_wdt_add(NULL);
    esp_task_wdt_reset();
}
 
