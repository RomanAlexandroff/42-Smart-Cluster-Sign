/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ota_rollback.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 10:30:00 by raleksan          #+#    #+#             */
/*   Updated: 2026/06/30 10:30:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/*                                                                            */
/*   OTA rollback functionality automatically restores previous stable        */
/*   version of the firmware in case a newly downloaded firmware happens      */
/*   to contain a bug that makes the device stall or crash. OTA rollback      */
/*   is a safety feature that prevents an update from bricking the device.    */
/*                                                                            */
/*   DO NOT CHANGE ANYTHING IN THIS FILE UNLES YOU ARE ABSOLUTELY CERTAIN     */
/*   THAT YOU KNOW WHAT YOU ARE DOING. A SINGLE MISSED MISTAKE IN THIS FILE   */
/*   MAY RENDER THE FUTURE OTA UPDATES OF THE AFFECTED DEVICES IMPOSSIBLE.    */
/*                                                                            */
/* ************************************************************************** */

#include <esp_ota_ops.h>
#include <esp_partition.h>
#include "42-Smart-Cluster-Sign.h"

/*
*   Device reset does not not always happen because of a crash. This
*   function detects the device reset reason and prevents firmware
*   rollback if the reset was caused by a normal device behaviour.
*/
static bool bad_reset_reason(void)
{
    esp_reset_reason_t reason;

    reason = esp_reset_reason();
    return (reason == ESP_RST_PANIC || reason == ESP_RST_TASK_WDT
        || reason == ESP_RST_INT_WDT || reason == ESP_RST_WDT);
}

/*
*   This function switches the next boot to the other OTA app
*   partition. If the device is running from app0, it selects app1;
*   if it is running from app1, it selects app0. The device restarts
*   immediately after the boot partition is changed.
*/
void rollback_firmware_update(void)
{
    const esp_partition_t *running_partition;
    const esp_partition_t *rollback_partition;
    esp_err_t             result;

    if (!rtc_g.defective_firmware)
    {
        rtc_g.defective_firmware = true;
        return ;
    }
    if (!bad_reset_reason())
    {
        rtc_g.defective_firmware = true;
        return ;
    }
    running_partition = esp_ota_get_running_partition();
    if (running_partition == NULL)
        // TODO: Partition Guess Game - if you don't know which partition fails,
        // label them yourself and try running, then discart the one failing
        return ;
    rollback_partition = NULL;
    if (strcmp(running_partition->label, "app0") == 0)
        rollback_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
    else if (strcmp(running_partition->label, "app1") == 0)
        rollback_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
    else
        // TODO: Partition Guess Game - if you don't know which partition fails,
        // label them yourself and try running, then discart the one failing
        return ;
    if (rollback_partition == NULL)
        // TODO: Partition Guess Game - if you don't know which partition fails,
        // label them yourself and try running, then discart the one failing
        return ;
    result = esp_ota_set_boot_partition(rollback_partition);
    if (result != ESP_OK)
        return ;
    rtc_g.defective_firmware = false;
    delay(1000);
    ESP.restart();
}
 
