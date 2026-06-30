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

static bool rollback_is_pending_verify(void)
{
    const esp_partition_t  *running;
    esp_ota_img_states_t   ota_state;

    running = esp_ota_get_running_partition();
    if (running == NULL)
        return (false);
    if (esp_ota_get_state_partition(running, &ota_state) != ESP_OK)
        return (false);
    return (ota_state == ESP_OTA_IMG_PENDING_VERIFY);
}

/*
*   This function confirms to the bootloader that the newly
*   downloaded firmware is in a fully working state and that
*   it is safe to keep it permanently. If any kind of reset
*   happens before execution reaches this function,
*   the bootloader will consider the newly downloaded firmware
*   disfunctional and rollback to the previous firmware.
*/
void  confirm_valid_firmware(void)
{
    if (!rollback_is_pending_verify())
        return;
    if (battery_check() <= (BATTERY_GOOD + BATTERY_CRITICAL) / 2)
        return;
    DEBUG_PRINTF("[ROLLBACK] Firmware is pending verification\n");
    if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK)
        DEBUG_PRINTF("[ROLLBACK] Firmware marked as valid\n");
    else
        DEBUG_PRINTF("[ROLLBACK] Failed to mark firmware as valid\n");
}

/*
*   Calling this function tells the bootloader that the newly
*   downloaded firmware has malfunctions and that the system
*   has to be instantly rolled back to the previous firmware.
*/
void  reject_firmware_and_reboot(void)
{
    DEBUG_PRINTF("[ROLLBACK] Firmware rejected, rolling back\n");
    esp_ota_mark_app_invalid_rollback_and_reboot();
}
 
