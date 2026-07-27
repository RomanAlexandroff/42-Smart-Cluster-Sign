
#include <Arduino.h>
#include <WiFi.h>
#include <esp_sleep.h>

#define MSEC_TO_USEC_FACTOR 1000ULL
#define FT_DELAY_MAX_SLEEP_LIMIT_MS 900000ULL
#define TCP_TEST_PORT 80
#define TCP_TEST_TIMEOUT_MS 3000
#define RECONNECT_TIMEOUT_MS 30000
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// ============================================================
// Sleep test definitions
// ============================================================

const uint32_t sleep_times[] =
{
    500,
    1000,
    2000,
    5000,
    10000,
    30000,
    60000,
    120000,
    300000,
    600000,
    900000
};

const char *sleep_names[] =
{
    "500 ms",
    "1 s",
    "2 s",
    "5 s",
    "10 s",
    "30 s",
    "1 min",
    "2 min",
    "5 min",
    "10 min",
    "15 min"
};

const uint8_t sleep_test_count = sizeof(sleep_times) / sizeof(sleep_times[0]);

// ============================================================
// Forward declarations
// ============================================================

void connect_wifi();
void print_wifi_info();
void print_system_info();
void print_separator();
void wifi_event_handler(WiFiEvent_t event);
bool tcp_test();
bool reconnect_wifi();
void run_test(uint32_t sleep_time, const char *name);
void print_menu();
void process_menu();

void watchdog_start();
void watchdog_stop();
void ft_delay(uint64_t time_in_millis);

// ============================================================
// Setup
// ============================================================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    print_separator();
    Serial.println("ESP32-C3 Light Sleep Wi-Fi Experiment");
    print_separator();

    print_system_info();

    WiFi.onEvent(wifi_event_handler);

    connect_wifi();

    print_menu();
}


// ============================================================
// Loop
// ============================================================

void loop()
{
    process_menu();
}


// ============================================================
// Wi-Fi
// ============================================================

void connect_wifi()
{
    Serial.println();
    Serial.println("[WIFI] Connecting...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t start = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);

        if (millis() - start > 30000)
        {
            Serial.println("[WIFI] Connection timeout");
            return;
        }
    }

    Serial.println("[WIFI] Connected");
    print_wifi_info();
}


void wifi_event_handler(WiFiEvent_t event)
{
    Serial.print("[WIFI EVENT] ");

    switch (event)
    {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("STA_CONNECTED");
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("STA_DISCONNECTED");
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.println("STA_GOT_IP");
            break;

        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            Serial.println("STA_LOST_IP");
            break;

        default:
            break;
    }
}


// ============================================================
// Diagnostics
// ============================================================

void print_system_info()
{
    Serial.println();
    Serial.println("SYSTEM INFO");
    Serial.println("-----------------------------");

    Serial.print("Chip model: ");
    Serial.println(ESP.getChipModel());

    Serial.print("CPU frequency: ");
    Serial.print(getCpuFrequencyMhz());
    Serial.println(" MHz");

    Serial.print("Flash size: ");
    Serial.print(ESP.getFlashChipSize() / 1024);
    Serial.println(" KB");

    Serial.print("Free heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");

    Serial.println();
}


void print_wifi_info()
{
    Serial.println();
    Serial.println("WI-FI INFO");
    Serial.println("-----------------------------");

    Serial.print("Mode: ");
    Serial.println(WiFi.getMode());

    Serial.print("Status: ");
    Serial.println(WiFi.status());

    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    Serial.print("BSSID: ");
    Serial.println(WiFi.BSSIDstr());

    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());

    Serial.println();
}


void print_separator()
{
    Serial.println();
    Serial.println("================================================");
}


// ============================================================
// TCP test
// ============================================================

bool tcp_test()
{
    WiFiClient client;

    client.setTimeout(TCP_TEST_TIMEOUT_MS);

    IPAddress gateway = WiFi.gatewayIP();

    Serial.print("[TCP TEST] Connecting to gateway ");

    Serial.print(gateway);

    Serial.print(":");

    Serial.print(TCP_TEST_PORT);

    Serial.println();

    uint32_t start = millis();

    bool result = client.connect(gateway, TCP_TEST_PORT);

    uint32_t elapsed = millis() - start;

    if (result)
    {
        Serial.print("[TCP TEST] PASS (");
        Serial.print(elapsed);
        Serial.println(" ms)");

        client.stop();

        return true;
    }

    Serial.print("[TCP TEST] FAIL (");
    Serial.print(elapsed);
    Serial.println(" ms)");

    return false;
}


// ============================================================
// Reconnect
// ============================================================

bool reconnect_wifi()
{
    Serial.println("[WIFI] Attempting reconnect...");

    uint32_t start = millis();

    WiFi.reconnect();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);

        if (millis() - start > RECONNECT_TIMEOUT_MS)
        {
            Serial.println("[WIFI] Reconnect timeout");
            return false;
        }
    }

    Serial.print("[WIFI] Reconnected in ");
    Serial.print(millis() - start);
    Serial.println(" ms");

    return true;
}


// ============================================================
// Sleep test execution
// ============================================================

void run_test(uint32_t sleep_time, const char *name)
{
    print_separator();

    Serial.print("SLEEP TEST: ");
    Serial.println(name);

    Serial.print("Requested sleep time: ");
    Serial.print(sleep_time);
    Serial.println(" ms");

    print_separator();

    Serial.println("[BEFORE SLEEP]");
    print_wifi_info();

    bool before_tcp = tcp_test();

    Serial.print("TCP before sleep: ");
    Serial.println(before_tcp ? "PASS" : "FAIL");

    Serial.println();
    Serial.println("Entering Light Sleep...");

    uint32_t start_time = millis();

    ft_delay(sleep_time);

    uint32_t actual_sleep_time = millis() - start_time;

    Serial.println();
    Serial.println("Awake!");

    Serial.print("Actual sleep duration: ");
    Serial.print(actual_sleep_time);
    Serial.println(" ms");

    print_separator();

    Serial.println("[AFTER SLEEP]");
    print_wifi_info();

    bool after_tcp = tcp_test();

    Serial.print("TCP after sleep: ");
    Serial.println(after_tcp ? "PASS" : "FAIL");


    if (!after_tcp)
    {
        Serial.println();
        Serial.println("[RECOVERY] Connection failed.");

        uint32_t reconnect_start = millis();

        if (reconnect_wifi())
        {
            Serial.print("[RECOVERY] Total reconnect time: ");
            Serial.print(millis() - reconnect_start);
            Serial.println(" ms");

            tcp_test();
        }
        else
        {
            Serial.println("[RECOVERY] Reconnect failed.");
        }
    }

    print_separator();
    Serial.println("TEST COMPLETE");
    print_separator();

    delay(2000);
}


// ============================================================
// Serial menu
// ============================================================

void print_menu()
{
    Serial.println();
    Serial.println("AVAILABLE TESTS");
    Serial.println("-----------------------------");

    for (uint8_t i = 0; i < sleep_test_count; i++)
    {
        Serial.print(i + 1);
        Serial.print(" - ");
        Serial.println(sleep_names[i]);
    }

    Serial.println("A - Run all tests");

    Serial.println();
    Serial.print("Selection: ");
}


void process_menu()
{
    if (!Serial.available())
        return;

    char input = Serial.read();

    if (input == '\n' || input == '\r')
        return;


    if (input == 'a' || input == 'A')
    {
        Serial.println("Running all tests...");

        for (uint8_t i = 0; i < sleep_test_count; i++)
        {
            run_test(sleep_times[i], sleep_names[i]);
        }

        print_menu();

        return;
    }


    if (input >= '1' && input <= '9')
    {
        uint8_t index = input - '1';

        run_test(
            sleep_times[index],
            sleep_names[index]
        );

        print_menu();

        return;
    }


    if (input == '0')
    {
        run_test(
            sleep_times[9],
            sleep_names[9]
        );

        print_menu();

        return;
    }


    Serial.println();
    Serial.println("Invalid selection");

    print_menu();
}


// ============================================================
// Watchdog placeholders
// ============================================================

void watchdog_start()
{
}


void watchdog_stop()
{
}


// ============================================================
// ft_delay()
// ============================================================

void ft_delay(uint64_t time_in_millis)
{
    if (time_in_millis <= 1)
    {
        delay(time_in_millis);
        return;
    }

    watchdog_stop();

    if (time_in_millis > FT_DELAY_MAX_SLEEP_LIMIT_MS)
        time_in_millis = FT_DELAY_MAX_SLEEP_LIMIT_MS;


    esp_sleep_enable_timer_wakeup(
        time_in_millis * MSEC_TO_USEC_FACTOR
    );


    if (esp_light_sleep_start() != ESP_OK)
    {
        Serial.println("[FT_DELAY] Light sleep failed.");

        delay(time_in_millis);
    }

    watchdog_start();
}