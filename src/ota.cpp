/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ota.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/19 12:50:00 by raleksan          #+#    #+#             */
/*   Updated: 2026/06/28 14:35:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/*                                                                            */
/*   Cloud pull OTA implementation using GitHub manifest + GitHub Releases.   */
/*                                                                            */
/*   This file is intentionally designed to contain everything related to     */
/*   the OTA functionality in order to prevent unwanted alterations during    */
/*   the future development of the project.                                   */
/*                                                                            */
/*   General description of the OTA pipeline:                                 */
/*      0. Connect Wi-Fi if needed.                                           */
/*      1. Download manifest.json from raw GitHub.                            */
/*      2. Parse JSON.                                                        */
/*      3. Compare with DEVICE_NAME.                                          */
/*      4. Select devices[DEVICE_NAME], otherwise select default device.      */
/*      5. If manifest "enabled" set to "false", stop.                        */
/*      6. If manifest software version <= SOFTWARE_VERSION, stop.            */
/*      7. If manifest firmware size > ESP.getFreeSketchSpace(), stop.        */
/*      8. If manifest firmware size != firmware .bin file size, stop.        */
/*      9. Download firmware .bin.                                            */
/*      10. Compute SHA-256 while streaming.                                  */
/*      11. If SHA-256 matches, call Update.end(true), otherwise stop.        */
/*      12. Reboot.                                                           */
/*                                                                            */
/* ************************************************************************** */

#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include <mbedtls/sha256.h>
#include "42-Smart-Cluster-Sign.h"

#define OTA_MANIFEST_URL        "https://raw.githubusercontent.com/RomanAlexandroff/42-Smart-Cluster-Sign/cloud_pull_ota_update_feature/ota/manifest.json"
#define OTA_HTTP_TIMEOUT_MS     15000
#define OTA_BUFFER_SIZE         1024

typedef enum e_ota_result
{
    OTA_RESULT_NO_UPDATE = 0,
    OTA_RESULT_UPDATED_REBOOTING = 1,
    OTA_RESULT_ERR_WIFI = -1,
    OTA_RESULT_ERR_MANIFEST_DOWNLOAD = -2,
    OTA_RESULT_ERR_MANIFEST_PARSE = -3,
    OTA_RESULT_ERR_MANIFEST_INVALID = -4,
    OTA_RESULT_ERR_DISABLED = -5,
    OTA_RESULT_ERR_NOT_ENOUGH_SPACE = -6,
    OTA_RESULT_ERR_FIRMWARE_DOWNLOAD = -7,
    OTA_RESULT_ERR_UPDATE_BEGIN = -8,
    OTA_RESULT_ERR_UPDATE_WRITE = -9,
    OTA_RESULT_ERR_UPDATE_END = -10,
    OTA_RESULT_ERR_SHA256 = -11
}   OTA_RESULT_t;

typedef struct s_ota_target
{
    String      version;
    String      url;
    String      sha256;
    uint32_t    size;
    bool        enabled;
    bool        device_specific;
}   OTA_TARGET_t;


/*
*   This function converts a software version string into an
*   integer value suitable for comparison. For example, "4.34"
*   becomes 434. The function expects a simple major.minor
*   version format and does not handle semantic versions.
*/
static uint32_t SW_ver_to_int(const String &version)
{
    int dot;
    uint32_t major;
    uint32_t minor;

    dot = version.indexOf('.');
    if (dot < 0)
        return (version.toInt() * 100);
    major = version.substring(0, dot).toInt();
    minor = version.substring(dot + 1).toInt();
    return (major * 100 + minor);
}


static void ota_send_telegram(const String &message)
{
    if (strlen(rtc_g.chat_id) > 0)
        bot.sendMessage(String(rtc_g.chat_id), message, "");
}


static bool ota_ensure_wifi(void)
{
    if (WiFi.status() == WL_CONNECTED)
        return true;
    wifi_connect();
    return (WiFi.status() == WL_CONNECTED);
}


/*
*   This function downloads a text file from the given HTTPS URL
*   and stores its content in the output String. It is used for
*   downloading the OTA manifest. Certificate validation is currently
*   disabled with setInsecure().
*/
static bool ota_download_text(const char *url, String &output)
{
    WiFiClientSecure client;
    HTTPClient       http;
    int              code;

    output = "";
    client.setInsecure();
    http.setTimeout(OTA_HTTP_TIMEOUT_MS);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    if (!http.begin(client, url))
        return (false);
    code = http.GET();
    if (code != HTTP_CODE_OK)
    {
        DEBUG_PRINTF("[OTA] HTTP GET failed, code: %d\n", code);
        http.end();
        return (false);
    }
    output = http.getString();
    http.end();
    return (output.length() > 0);
}


/*
*   This function extracts one firmware target entry from the parsed
*   manifest JSON. It reads version, URL, SHA-256 hash, file size
*   and enabled flag. The function returns false if any required
*   value is missing or invalid.
*/
static bool ota_manifest_target_from_variant(JsonVariant variant, OTA_TARGET_t &target, bool device_specific)
{
    const char* version;
    const char* url;
    const char* sha256;

    if (variant.isNull())
        return (false);
    version = variant["version"] | "";
    url = variant["url"] | "";
    sha256 = variant["sha256"] | "";
    target.version = String(version);
    target.url = String(url);
    target.sha256 = String(sha256);
    target.size = variant["size"] | 0;
    target.enabled = variant["enabled"] | false;
    target.device_specific = device_specific;
    if (target.version.length() == 0)
        return (false);
    if (target.url.length() == 0)
        return (false);
    if (target.sha256.length() != 64)
        return (false);
    if (target.size == 0)
        return (false);
    return (true);
}


/*
*   This function parses the downloaded manifest and selects the
*   correct firmware target for this device. It first looks for a
*   device-specific entry. If none is found, it falls back to the
*   default manifest entry.
*/
static bool ota_parse_manifest(const String &manifest_json, const String &device_id, OTA_TARGET_t &target)
{
    StaticJsonDocument<4096> doc;
    DeserializationError     error;
    JsonVariant              selected;

    error = deserializeJson(doc, manifest_json);
    if (error)
    {
        DEBUG_PRINTF("[OTA] JSON parse error: %s\n", error.c_str());
        return (false);
    }
    selected = doc["devices"][device_id];
    if (!selected.isNull())
    {
        DEBUG_PRINTF("[OTA] Device-specific manifest entry found\n");
        return (ota_manifest_target_from_variant(selected, target, true));
    }
    DEBUG_PRINTF("[OTA] No device-specific entry, using default\n");
    selected = doc["default"];
    return (ota_manifest_target_from_variant(selected, target, false));
}


/*
*   This function converts a 32-byte SHA-256 hash into a lowercase
*   hexadecimal string. The output buffer must have space for
*   64 characters plus the final null terminator.
*/
static void ota_sha256_to_hex(const uint8_t hash[32], char output[65])
{
    static const char* hex = "0123456789abcdef";
    uint8_t           i;

    i = 0;
    while (i < 32)
    {
        output[i * 2] = hex[(hash[i] >> 4) & 0x0F];
        output[i * 2 + 1] = hex[hash[i] & 0x0F];
        i++;
    }
    output[64] = '\0';
}


/*
*   This function downloads the firmware binary from GitHub Releases
*   and writes it into the inactive OTA partition. While downloading,
*   it calculates SHA-256 and later compares it with the manifest.
*   The update is finalized only if size and hash checks pass.
*   Calling function is responsible for pausing the watchdog.
*/
static bool ota_download_and_flash(const OTA_TARGET_t &target)
{
    WiFiClientSecure       client;
    HTTPClient             http;
    WiFiClient*            stream;
    int                    status_code;
    int                    content_length;
    uint8_t                buffer[OTA_BUFFER_SIZE];
    size_t                 available;
    uint32_t               last_progress;
    int                    read_bytes;
    size_t                 written;
    mbedtls_sha256_context sha_ctx;
    uint8_t                sha_result[32];
    char                   sha_hex[65];

/* INITIAL SETUP */
    client.setInsecure();
    http.setTimeout(OTA_HTTP_TIMEOUT_MS);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    DEBUG_PRINTF("[OTA] Firmware URL: %s\n", target.url.c_str());

/* CONNECTING TO SERVER AND GATHERING SIZE DATA */
    if (!http.begin(client, target.url))
    {
        DEBUG_PRINTF("[OTA] Failed to begin firmware HTTP request\n");
        return (false);
    }
    status_code = http.GET();
    if (status_code != HTTP_CODE_OK)
    {
        DEBUG_PRINTF("[OTA] Firmware HTTP GET failed, code: %d\n", status_code);
        http.end();
        return (false);
    }

/* CHECKING IF FIRMWARE SIZE MATCHES */
    content_length = http.getSize();
    DEBUG_PRINTF("[OTA] Manifest firmware size: %u\n", target.size);
    DEBUG_PRINTF("[OTA] HTTP content length: %d\n", content_length);
    DEBUG_PRINTF("[OTA] Free sketch space: %u\n", ESP.getFreeSketchSpace());
    if (content_length > 0 && (uint32_t)content_length != target.size)
    {
        DEBUG_PRINTF("[OTA] Size mismatch between manifest and HTTP header\n");
        http.end();
        return (false);
    }
    if (target.size > ESP.getFreeSketchSpace())
    {
        DEBUG_PRINTF("[OTA] Not enough OTA space\n");
        http.end();
        return (false);
    }

/* READING FIRMWARE FROM SERVER AND WRITING IT INTO MEMORY */
    if (!Update.begin(target.size))
    {
        DEBUG_PRINTF("[OTA] Update.begin() failed. Error: %s\n",
            Update.errorString());
        http.end();
        return (false);
    }
    mbedtls_sha256_init(&sha_ctx);
    mbedtls_sha256_starts(&sha_ctx, 0);
    stream = http.getStreamPtr();
    written = 0;
    last_progress = millis();                                         // Download stall detection
    while (http.connected() && written < target.size)
    {
        available = stream->available();
        if (available == 0)
        {
            if (millis() - last_progress > OTA_HTTP_TIMEOUT_MS)       // Download stall detection
            {
                DEBUG_PRINTF("[OTA] Download stalled\n");
                Update.abort();
                http.end();
                mbedtls_sha256_free(&sha_ctx);
                return (false);
            }
            delay(1);
            continue;
        }
        last_progress = millis();                                     // Download stall detection
        if (available > OTA_BUFFER_SIZE)
            available = OTA_BUFFER_SIZE;
        if (available > target.size - written)
            available = target.size - written;
        read_bytes = stream->readBytes(buffer, available);
        if (read_bytes <= 0)
        {
            DEBUG_PRINTF("[OTA] Stream read failed\n");
            Update.abort();
            http.end();
            mbedtls_sha256_free(&sha_ctx);
            return (false);
        }
        mbedtls_sha256_update(&sha_ctx, buffer, read_bytes);
        if (Update.write(buffer, read_bytes) != (size_t)read_bytes)
        {
            DEBUG_PRINTF("[OTA] Update.write() failed. Error: %s\n",
                Update.errorString());
            Update.abort();
            http.end();
            mbedtls_sha256_free(&sha_ctx);
            return (false);
        }
        written += read_bytes;
        DEBUG_PRINTF("[OTA] Written %u / %u bytes\n", (unsigned int)written, (unsigned int)target.size);
    }
    http.end();

/* CHECKING FOR ANY POSSIBLE FAILS */
    mbedtls_sha256_finish(&sha_ctx, sha_result);
    mbedtls_sha256_free(&sha_ctx);
    ota_sha256_to_hex(sha_result, sha_hex);
    DEBUG_PRINTF("[OTA] Expected SHA-256: %s\n", target.sha256.c_str());
    DEBUG_PRINTF("[OTA] Actual SHA-256:   %s\n", sha_hex);
    if (!target.sha256.equalsIgnoreCase(String(sha_hex)))
    {
        DEBUG_PRINTF("[OTA] SHA-256 mismatch\n");
        Update.abort();
        return (false);
    }
    if (written != target.size)
    {
        DEBUG_PRINTF("[OTA] Written size mismatch\n");
        Update.abort();
        return (false);
    }
    if (!Update.end(true))
    {
        DEBUG_PRINTF("[OTA] Update.end() failed. Error: %s\n",
            Update.errorString());
        return (false);
    }
    if (!Update.isFinished())
    {
        DEBUG_PRINTF("[OTA] Update not finished\n");
        return (false);
    }
    return (true);
}


/*
*   This function performs the full cloud-pull OTA workflow. It
*   downloads the manifest, selects the target firmware, compares
*   versions, checks firmware size, downloads and verifies the binary,
*   then reboots the device if the update succeeds.
*/
static OTA_RESULT_t ota_check_and_update(void)
{
    String          manifest_json;
    String          device_id;
    String          current_version;
    OTA_TARGET_t    target;

/* INITIAL CHECKS AND INFORMATION GATHERING */
    DEBUG_PRINTF("\n[OTA] Cloud pull OTA started\n");
    if (!ota_ensure_wifi())
    {
        DEBUG_PRINTF("[OTA] Wi-Fi unavailable\n");
        return (OTA_RESULT_ERR_WIFI);
    }
    watchdog_reset();
    device_id = String(DEVICE_NAME);
    current_version = String(SOFTWARE_VERSION, 2);
    DEBUG_PRINTF("[OTA] Device ID: %s\n", device_id.c_str());
    DEBUG_PRINTF("[OTA] Current firmware version: %s\n", current_version.c_str());

/* DOWNLOADING THE MANIFEST FROM GITHUB REPOSITORY */
    if (!ota_download_text(OTA_MANIFEST_URL, manifest_json))
    {
        DEBUG_PRINTF("[OTA] Failed to download manifest\n");
        return (OTA_RESULT_ERR_MANIFEST_DOWNLOAD);
    }
    DEBUG_PRINTF("[OTA] Manifest downloaded, %u bytes\n", (unsigned int)manifest_json.length());

/* PARSING OF THE MANIFEST TO GET UPDATE INFORMATION */
    if (!ota_parse_manifest(manifest_json, device_id, target))
    {
        DEBUG_PRINTF("[OTA] Failed to parse/select manifest target\n");
        return (OTA_RESULT_ERR_MANIFEST_PARSE);
    }
    DEBUG_PRINTF("[OTA] Target version: %s\n", target.version.c_str());
    DEBUG_PRINTF("[OTA] Target enabled: %s\n", target.enabled ? "true" : "false");
    DEBUG_PRINTF("[OTA] Device-specific: %s\n", target.device_specific ? "true" : "false");

/* CHECKING THE "TARGET ENABLED" FLAG FROM THE MANIFEST */
    if (!target.enabled)
    {
        DEBUG_PRINTF("[OTA] Selected update entry is disabled\n");
        return (OTA_RESULT_ERR_DISABLED);
    }

/* CHECKING THE SOFTWARE VERSION FROM THE MANIFEST */
    if (SW_ver_to_int(target.version) <= SW_ver_to_int(current_version))
    {
        DEBUG_PRINTF("[OTA] No update needed\n");
        return (OTA_RESULT_NO_UPDATE);
    }

/* CHECKING IF THE SOFTWARE FITS INTO THE MEMORY */
    if (target.size > ESP.getFreeSketchSpace())
    {
        DEBUG_PRINTF("[OTA] Firmware too large for OTA partition\n");
        return (OTA_RESULT_ERR_NOT_ENOUGH_SPACE);
    }

/* PROCEED TO DOWNLOADING THE SOFTWARE AND FLASHING THE DEVICE */
    ota_send_telegram("OTA update found. Downloading firmware...");
    display_cluster_number(OTA_WAITING);
    watchdog_stop();
    if (!ota_download_and_flash(target))
    {
        watchdog_start();
        display_cluster_number(OTA_FAIL);
        ota_send_telegram("OTA update failed.");
        ft_delay(3000);
        clear_display();
        return (OTA_RESULT_ERR_FIRMWARE_DOWNLOAD);
    }
    display_cluster_number(OTA_SUCCESS);
    ota_send_telegram("OTA update installed. Rebooting...");
    delay(3000);
    ESP.restart();
    return (OTA_RESULT_UPDATED_REBOOTING);
}


/*
*   This function is an API of the OTA functionality for
*   the rest of the program. It decides whether to run
*   the updating or not whether on schedule or on request. 
*/
void ota_handling(void)
{
    const uint8_t wakeup_hour[] = {WAKE_UP_HOURS};
    bool          scheduled_ota;

    scheduled_ota = com_g.hour <= wakeup_hour[0] &&
                      (com_g.day == 3 || com_g.day == 11 ||
                        com_g.day == 19 || com_g.day == 27);
    if (com_g.ota)
        ota_send_telegram("OTA check started.");
    if (scheduled_ota || com_g.ota)
        ota_check_and_update();
    com_g.ota = false;
}
 
