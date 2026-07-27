/*
HOW TO RUN THE TEST:

1. In Arduino IDE, select XIAO_ESP32C3.
2. Select an OTA-capable partition scheme. Avoid anything that says No OTA.
3. Set:
		#define TEST_BUILD_ID 1
		#define PENDING_ACTION ACTION_MARK_VALID
4. Upload this first build over USB.
5. Open Serial Monitor at 115200.
6. Confirm it starts AP rollback-test.
7. Now change only these lines:
		#define TEST_BUILD_ID 2
		#define PENDING_ACTION ACTION_RESTART_WITHOUT_VALIDATION
8. Use Sketch -> Export Compiled Binary.
9. Connect your computer to Wi-Fi AP rollback-test, password 12345678.
10. Open http://192.168.4.1/.
11. Upload the generated .bin for build 2.
12. Watch Serial Monitor.


EXPECTED RESULTS:

If bootloader rollback works:
- Build 2 boots
- state: ESP_OTA_IMG_PENDING_VERIFY
- ACTION_RESTART_WITHOUT_VALIDATION
- ESP.restart()
- Bootloader rolls back
- Build 1 boots again

If rollback is unavailable:
- Build 2 boots
- state is not ESP_OTA_IMG_PENDING_VERIFY
- or build 2 keeps booting after restart


THEN REPEAT THE TEST with changing the PENDING_ACTION for the build 2:

		#define TEST_BUILD_ID 2
		#define PENDING_ACTION ACTION_EXPLICIT_INVALID_ROLLBACK

If bootloader rollback works, esp_ota_mark_app_invalid_rollback_and_reboot()
should reboot into build 1. If it returns an error or stays on build 2, that
strongly suggests Arduino’s bootloader config for this board/core does not 
ssupport rollback.
*/




#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_system.h>
#define ACTION_MARK_VALID 0
#define ACTION_RESTART_WITHOUT_VALIDATION 1
#define ACTION_EXPLICIT_INVALID_ROLLBACK 2



// BUILD 1: UNCOMMENT THIS 2 LINES AND COMMENT OUT OTHER BUILDS
//#define TEST_BUILD_ID 1
//#define PENDING_ACTION ACTION_MARK_VALID


// BUILD 2 - OPTION 1: UNCOMMENT THIS 2 LINES AND COMMENT OUT OTHER BUILDS
#define TEST_BUILD_ID 2
#define PENDING_ACTION ACTION_RESTART_WITHOUT_VALIDATION


// BUILD 2 - OPTION 2: UNCOMMENT THIS 2 LINES AND COMMENT OUT OTHER BUILDS
//#define TEST_BUILD_ID 2
//#define PENDING_ACTION ACTION_EXPLICIT_INVALID_ROLLBACK



// THIS TINY BLOCK OF C CODE HAS CHANGED EVERYTHING !!!!!
extern "C" bool verifyRollbackLater(void)
{
    return true;
}


WebServer server(80);

static const char *state_to_string(esp_ota_img_states_t state)
{
    switch (state)
    {
        case ESP_OTA_IMG_NEW: return "ESP_OTA_IMG_NEW";
        case ESP_OTA_IMG_PENDING_VERIFY: return "ESP_OTA_IMG_PENDING_VERIFY";
        case ESP_OTA_IMG_VALID: return "ESP_OTA_IMG_VALID";
        case ESP_OTA_IMG_INVALID: return "ESP_OTA_IMG_INVALID";
        case ESP_OTA_IMG_ABORTED: return "ESP_OTA_IMG_ABORTED";
        case ESP_OTA_IMG_UNDEFINED: return "ESP_OTA_IMG_UNDEFINED";
        default: return "UNKNOWN";
    }
}

static void print_partition_info(const char *name, const esp_partition_t *p)
{
    if (!p)
    {
        Serial.printf("%s: NULL\n", name);
        return;
    }

    Serial.printf("%s: label=%s subtype=0x%02X address=0x%06lX size=%lu\n",
                  name, p->label, p->subtype, p->address, p->size);

    esp_ota_img_states_t state;
    esp_err_t err = esp_ota_get_state_partition(p, &state);
    if (err == ESP_OK)
        Serial.printf("%s state: %s\n", name, state_to_string(state));
    else
        Serial.printf("%s state: esp_ota_get_state_partition() failed: %s\n",
                      name, esp_err_to_name(err));
}

static void print_ota_info(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_partition_t *boot = esp_ota_get_boot_partition();
    const esp_partition_t *next = esp_ota_get_next_update_partition(NULL);
    const esp_partition_t *last_invalid = esp_ota_get_last_invalid_partition();

    Serial.println();
    Serial.println("========== OTA INFO ==========");
    Serial.printf("TEST_BUILD_ID: %d\n", TEST_BUILD_ID);
    Serial.printf("PENDING_ACTION: %d\n", PENDING_ACTION);
    Serial.printf("Reset reason: %d\n", esp_reset_reason());

    print_partition_info("Running partition", running);
    print_partition_info("Boot partition", boot);
    print_partition_info("Next update partition", next);
    print_partition_info("Last invalid partition", last_invalid);

    Serial.println("==============================");
    Serial.println();
}

static esp_ota_img_states_t get_running_state(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t state = ESP_OTA_IMG_UNDEFINED;

    if (!running)
        return ESP_OTA_IMG_UNDEFINED;

    esp_err_t err = esp_ota_get_state_partition(running, &state);
    if (err != ESP_OK)
    {
        Serial.printf("Could not read running state: %s\n", esp_err_to_name(err));
        return ESP_OTA_IMG_UNDEFINED;
    }

    return state;
}

static void handle_pending_verify(void)
{
    esp_ota_img_states_t state = get_running_state();

    if (state != ESP_OTA_IMG_PENDING_VERIFY)
    {
        Serial.println("Running app is NOT pending verification.");
        Serial.println("If this is the newly OTA-uploaded build, bootloader rollback is probably not enabled.");
        return;
    }

    Serial.println("Running app is PENDING_VERIFY.");
    Serial.println("This means bootloader rollback support is active enough to test.");

#if PENDING_ACTION == ACTION_MARK_VALID
    Serial.println("ACTION_MARK_VALID: marking this firmware valid.");
    esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
    Serial.printf("esp_ota_mark_app_valid_cancel_rollback(): %s\n", esp_err_to_name(err));

#elif PENDING_ACTION == ACTION_RESTART_WITHOUT_VALIDATION
    Serial.println("ACTION_RESTART_WITHOUT_VALIDATION.");
    Serial.println("If rollback works, after restart the bootloader should return to the previous build.");
    delay(5000);
    ESP.restart();

#elif PENDING_ACTION == ACTION_EXPLICIT_INVALID_ROLLBACK
    Serial.println("ACTION_EXPLICIT_INVALID_ROLLBACK.");
    Serial.println("Calling esp_ota_mark_app_invalid_rollback_and_reboot().");
    delay(3000);
    esp_err_t err = esp_ota_mark_app_invalid_rollback_and_reboot();
    Serial.printf("esp_ota_mark_app_invalid_rollback_and_reboot() returned: %s\n", esp_err_to_name(err));
    Serial.println("If this returned instead of rebooting, rollback probably failed/unavailable.");

#else
    Serial.println("Unknown PENDING_ACTION.");
#endif
}

static void handle_root(void)
{
    String html;
    html += "<!doctype html><html><body>";
    html += "<h2>ESP32 Bootloader Rollback Test</h2>";
    html += "<p>Build ID: ";
    html += TEST_BUILD_ID;
    html += "</p>";
    html += "<form method='POST' action='/update' enctype='multipart/form-data'>";
    html += "<input type='file' name='firmware'>";
    html += "<input type='submit' value='Upload firmware'>";
    html += "</form>";
    html += "</body></html>";

    server.send(200, "text/html", html);
}

static void handle_update_finished(void)
{
    bool ok = !Update.hasError();

    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", ok ? "Update OK. Rebooting..." : "Update FAILED.");

    Serial.println(ok ? "HTTP update OK. Rebooting..." : "HTTP update failed.");
    delay(1000);

    if (ok)
        ESP.restart();
}

static void handle_update_upload(void)
{
    HTTPUpload &upload = server.upload();

    if (upload.status == UPLOAD_FILE_START)
    {
        Serial.printf("Upload start: %s\n", upload.filename.c_str());

        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
        {
            Serial.printf("Update.begin failed: %s\n", Update.errorString());
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        size_t written = Update.write(upload.buf, upload.currentSize);
        if (written != upload.currentSize)
        {
            Serial.printf("Update.write failed: %s\n", Update.errorString());
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
        {
            Serial.printf("Update complete: %u bytes\n", upload.totalSize);
        }
        else
        {
            Serial.printf("Update.end failed: %s\n", Update.errorString());
        }
    }
}

static void start_uploader_ap(void)
{
    const char *ssid = "rollback-test";
    const char *password = "12345678";

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    Serial.println();
    Serial.println("Uploader AP started.");
    Serial.printf("SSID: %s\n", ssid);
    Serial.printf("Password: %s\n", password);
    Serial.printf("Open: http://%s/\n", WiFi.softAPIP().toString().c_str());

    server.on("/", HTTP_GET, handle_root);
    server.on("/update", HTTP_POST, handle_update_finished, handle_update_upload);
    server.begin();
}

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println();
    Serial.println("======================================");
    Serial.println("ESP32 BOOTLOADER ROLLBACK TEST");
    Serial.println("======================================");
    Serial.printf("verifyRollbackLater called: %s\n",
              rollback_later_hook_called ? "yes" : "no");
    print_ota_info();
    handle_pending_verify();
    start_uploader_ap();
}

void loop()
{
    server.handleClient();
    delay(5);
}
