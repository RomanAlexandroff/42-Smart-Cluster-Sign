/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ota_rollback.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 10:30:00 by raleksan          #+#    #+#             */
/*   Updated: 2026/07/19 09:00:00 by raleksan         ###   ########.fr       */
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

/*
*   For the OTA to remain functionable, correct Wi-Fi credentials
*   are absolutely crucial. One little typo - e.g. replacing 
*   a single capital letter with lowercase in the password - will
*   render the whole device disfunctional AND also will cut off
*   the posibility to fix it distantly via another OTA update.
*   This function checks the validity of the Wi-Fi credentials by
*   simply trying to connect to the given Wi-Fi network. If it
*   cannot connect, the credentials are considered to be incorrect
*   and the function prevents the firmware from being marked as
*   VERIFIED, which in turns causes the firmware rollback to
*   trigger after the deep slee.
*/
static bool wifi_credentials_test(void)
{
    return (ensure_wifi_connection());
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
*   Updates the persistent firmware rollback state. Checks if the caller is
*   trying to rewrite the current VERIFIED state with VERIFIED again to reduce
*   Flash wearing out from writing to it. The function INTENTIONALLY does not
*   check rewriting UNTRUSTWORTHY with UNTRUSTWORTHY because that would break
*   the logic of creating the file if it did not exist yet.
*/
void write_rollback_flag(FIRMWARE_t state)
{
    File file;

    if (state == VERIFIED && read_rollback_flag() == VERIFIED)         // do NOT optimise
        return ;
    file = LittleFS.open("/defective_firmware.txt", "w");
    if (!file)
        return ;
    file.print((bool)state ? "1" : "0");
    file.close();
}


/*
*   Makes the judgement call if the firmware to be trusted. When marking the
*   firmware as VERIFIED, Wi-Fi credentials are validated before the rollback
*   flag is cleared. In case it fails, instant firmware rollback is triggered.
*/
void set_rollback_flag(FIRMWARE_t new_state)
{
    if (new_state == VERIFIED && read_rollback_flag() == UNTRUSTWORTHY)
    {
        if (!wifi_credentials_test())
        {
            rollback_firmware_update();
            return ;                          // in case the previous function fails and returns here
        }
        rtc_g.firmware_verified = true;
    }
    write_rollback_flag(new_state);
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

    /* This if-block lowers Flash wearing out by reducing write operations.
    After a new firmware runs once and gets Verified, this flag locks out
    the rest of the function forever. A power loss or hard reset may make
    this flag loose its value, but the next if-block will fix it. The logic
    accounts for the fact that the value of the flag gets lost over updates
    of the firmware */
    if (rtc_g.firmware_verified)
        return ;

    /* This if-block runs only the very first cycle of a new firmware or
    after a power loss or hard reset. Running this if-block initializes
    a "test cycle" for the currently running firmware - no matter if it
    is new or old. Failing a "test cycle" does not necessarily mean that
    the firmware will be rolled back - the next if-block may give another 
    "test cycle" if conditions apply. */    
    if (read_rollback_flag() == VERIFIED)
    {
        write_rollback_flag(UNTRUSTWORTHY);
        return ;
    }

    /* This if-block judges if the "test cycle" was failed for a legitemate
    reason. For example, a brown-out or the User pressing reset button can't
    be a sign that the firmware is defective, but they still cause failure
    of a "test cycle". In such cases the firmware gets another "test cycle" */
    if (!bad_reset_reason() && wifi_credentials_test())
    {
        DEBUG_PRINTF("[ROLLBACK] Previous reset was not suspicious. Skipping rollback.\n");
        write_rollback_flag(UNTRUSTWORTHY);
        return ;
    }

    /* At this point the program is absolutely certain that the currently
    running firmware is defective and the device needs to be rolled back
    to the previous firmware version. */
    watchdog_reset();
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
    write_rollback_flag(VERIFIED);                                        // prevents the old firmware from being instantly rolled back
    delay(1000);
    ESP.restart();
}
 
