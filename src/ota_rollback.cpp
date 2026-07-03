/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ota_rollback.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 10:30:00 by raleksan          #+#    #+#             */
/*   Updated: 2026/07/03 09:00:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/*                                                                            */
/*   OTA rollback functionality automatically restores previous stable        */
/*   version of the firmware in case a newly downloaded firmware happens      */
/*   to contain a bug that makes the device stall or crash. OTA rollback      */
/*   is a safety feature that prevents an update from bricking the device.    */
/*                                                                            */
/*   This rollback implementation is based on a persistent LittleFS flag      */
/*   and reset reason detection. A newly installed firmware gets one full     */
/*   run cycle to prove itself. If the device reaches normal sleep, the       */
/*   firmware is marked verified. If it crashes or stalls before that, the    */
/*   next boot switches back to the other OTA application partition.          */
/*                                                                            */
/*   DO NOT CHANGE ANYTHING IN THIS FILE UNLES YOU ARE ABSOLUTELY CERTAIN     */
/*   THAT YOU KNOW WHAT YOU ARE DOING. A SINGLE MISSED MISTAKE IN THIS FILE   */
/*   MAY RENDER THE FUTURE OTA UPDATES OF THE AFFECTED DEVICES IMPOSSIBLE.    */
/*                                                                            */
/* ************************************************************************** */

#include <esp_ota_ops.h>
#include <esp_partition.h>
#include "42-Smart-Cluster-Sign.h"


void set_rollback_flag(FIRMWARE_t state)
{
    File file;

    file = LittleFS.open("/defective_firmware.txt", "w");
    if (!file)
        return ;
    file.print((bool)state ? "1" : "0");
    file.close();
}


/*
*   In case of any LittleFS failure, the rollback flag becomes
*   unavailable, so the function intentionally returns "true"
*   to let reset reason detection decide if to perform rollback.
*/
static FIRMWARE_t read_rollback_flag(void)
{
    File file;
    char value;

    if (!LittleFS.exists("/defective_firmware.txt"))
        return (UNTRUSTWORTHY);
    file = LittleFS.open("/defective_firmware.txt", "r");
    if (!file)
        return (UNTRUSTWORTHY);
    value = file.read();
    file.close();
    return ((FIRMWARE_t)(value == '1'));
}


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
void  rollback_firmware_update(void)
{
    const esp_partition_t *running_partition;
    const esp_partition_t *rollback_partition;
    esp_err_t             result;

    if (rtc_g.firmware_verified)
        return ;
    if (read_rollback_flag() == VERIFIED)
    {
        set_rollback_flag(UNTRUSTWORTHY);
        rtc_g.firmware_verified = true;
        return ;
    }
    if (!bad_reset_reason())
    {
        DEBUG_PRINTF("[ROLLBACK] Previous reset was not suspicious. Skipping rollback.\n");
        set_rollback_flag(UNTRUSTWORTHY);
        rtc_g.firmware_verified = true;
        return ;
    }
    set_rollback_flag(VERIFIED);
    running_partition = esp_ota_get_running_partition();
    if (running_partition == NULL)
    {
        DEBUG_PRINTF("[ROLLBACK] Failed to get running partition\n");
        return ;
    }
    DEBUG_PRINTF("[ROLLBACK] Running partition: %s at 0x%06lX\n", running_partition->label, running_partition->address);
    rollback_partition = NULL;
    if (strcmp(running_partition->label, "app0") == 0)
        rollback_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
    else if (strcmp(running_partition->label, "app1") == 0)
        rollback_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
    else
    {
        DEBUG_PRINTF("[ROLLBACK] Unknown running partition: %s\n", running_partition->label);
        return ;
    }
    if (rollback_partition == NULL)
    {
        DEBUG_PRINTF("[ROLLBACK] Failed to find rollback partition\n");
        return ;
    }
    DEBUG_PRINTF("[ROLLBACK] Switching boot partition to: %s at 0x%06lX\n",rollback_partition->label, rollback_partition->address);
    result = esp_ota_set_boot_partition(rollback_partition);
    if (result != ESP_OK)
    {
        DEBUG_PRINTF("[ROLLBACK] Failed to set boot partition. Error: %d\n", result);
        return ;
    }
    DEBUG_PRINTF("[ROLLBACK] Boot partition changed. Restarting...\n");
    delay(1000);
    ESP.restart();
}
 
