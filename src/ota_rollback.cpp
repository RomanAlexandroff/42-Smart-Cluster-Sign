/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ota_rollback.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 10:30:00 by raleksan          #+#    #+#             */
/*   Updated: 2026/07/24 13:30:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/*                                                                            */
/*   OTA rollback functionality automatically restores previous stable        */
/*   version of the firmware in case a newly downloaded firmware happens      */
/*   to contain a bug that makes the device stall or crash. OTA rollback      */
/*   is a safety feature that prevents an update from bricking the device.    */
/*                                                                            */
/*   A newly installed firmware gets one full run cycle to prove itself.      */
/*   If the device reaches normal sleep, the firmware is marked verified.     */
/*   If it crashes or stalls before that, the next boot switches back to      */
/*   the other OTA application partition.                                     */
/*                                                                            */
/*   DO NOT CHANGE ANYTHING IN THIS FILE UNLES YOU ARE ABSOLUTELY CERTAIN     */
/*   THAT YOU KNOW WHAT YOU ARE DOING. A SINGLE MISSED MISTAKE IN THIS FILE   */
/*   MAY RENDER THE FUTURE OTA UPDATES OF THE AFFECTED DEVICES IMPOSSIBLE.    */
/*                                                                            */
/* ************************************************************************** */

#include <esp_ota_ops.h>
#include "42-Smart-Cluster-Sign.h"


/*
*   For Arduino to stop blocking bootloder-level firmware
*   rollback. By default, Arduino auto-validates all the
*   new firmware. For beginners it helps prevent confusion
*   why their firmware suddenly changes on the fly. But for
*   advanced users it blocks bootloder-level rollbacks.
*   This function turns Arduino auto-validation off.
*/
extern "C" bool verifyRollbackLater(void)
{
    return (true);
}


/*
*   Tells if the currently running firmware has been
*   rolled-back into from another faulty firmware.
*/
static bool  firmware_rollback_happened(void)
{
    const esp_partition_t   *running;
    const esp_partition_t   *last_invalid;
    esp_ota_img_states_t    state;

    running = esp_ota_get_running_partition();
    last_invalid = esp_ota_get_last_invalid_partition();
    if (running == NULL || last_invalid == NULL)
        return (false);
    if (running->address == last_invalid->address)
        return (false);
    if (esp_ota_get_state_partition(last_invalid, &state) != ESP_OK)
        return (false);
    return (state == ESP_OTA_IMG_ABORTED || state == ESP_OTA_IMG_INVALID);
}


/*
*   Ensures that the Telegram chat gets notified
*   that the newly uploaded firmware has been
*   rolled-back to the previous firmware version,
*   only once - during the first execution cycle.
*/
void notify_firmware_rollback_once(esp_reset_reason_t reason)
{
    String message;

    if (!firmware_rollback_happened())
        return ;
    if (!(reason == ESP_RST_SW ||
            reason == ESP_RST_PANIC ||
            reason == ESP_RST_TASK_WDT ||
            reason == ESP_RST_INT_WDT ||
            reason == ESP_RST_WDT))
        return ;
    message = "Firmware Rollback has been executed. Returned to the firmware version ";
    message += String(SOFTWARE_VERSION, 2);
    send_telegram_message(message);
}


/*
*   Detects pending verification. This function
*   tells if the currently running firmware is
*   being in the process of verification or not.
*/
bool  firmware_being_tested(void)
{
    const esp_partition_t  *running;
    esp_ota_img_states_t   state;

    running = esp_ota_get_running_partition();
    if (running == NULL)
        return (false);
    if (esp_ota_get_state_partition(running, &state) != ESP_OK)
        return (false);
    return (state == ESP_OTA_IMG_PENDING_VERIFY);
}


/*
*   For the OTA to remain functionable, correct Wi-Fi
*   credentials are absolutely crucial. This function
*   checks the validity of the Wi-Fi credentials by
*   trying to connect to the given Wi-Fi network. If
*   it cannot connect, the credentials are considered
*   to be incorrect and firmware rollback is executed.
*   If the device had connection to download the new
*   firmware, but the new firmware cannot connect 1
*   second after that - credentials may be wrong.
*/
void  wifi_credentials_test(void)
{
    esp_err_t result;

    if (!firmware_being_tested())
        return ;
    if (!ensure_wifi_connection())
    {
        DEBUG_PRINTF("\n[OTA] Firmware failed the Wi-Fi credentials test. Rolling back...\n");
        result = esp_ota_mark_app_invalid_rollback_and_reboot();
        if (result != ESP_OK)
        {
            DEBUG_PRINTF("[OTA] Failed to Rollback! Reason: %d\n", result);
            com_g.block_validation = true;
        }
    }
}


void  set_firmware_verified(void)
{
    esp_err_t result;

    if (com_g.block_validation || !firmware_being_tested())
        return ;
    DEBUG_PRINTF("[OTA] Firmware is pending verification\n");
    result = esp_ota_mark_app_valid_cancel_rollback();
    if (result == ESP_OK)
        DEBUG_PRINTF("[OTA] Firmware marked valid\n");
    else
        DEBUG_PRINTF("[OTA] Failed to mark firmware valid: %d\n", result);
}
 
