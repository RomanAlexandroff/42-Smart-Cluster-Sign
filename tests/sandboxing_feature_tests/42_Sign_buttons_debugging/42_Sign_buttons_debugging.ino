
#include "header.h"

volatile unsigned long last_interrupt_diagnostics = 0;              // for a button debounce
volatile unsigned long last_interrupt_ota = 0;                      // for a button debounce
volatile unsigned long last_interrupt_warning = 0;                  // for a button debounce

# define DEBOUNCE_DELAY 1000UL                                      // for a button debounce


volatile bool          diagnostics_pressed = false;
volatile bool          ota_pressed = false;
volatile bool          warning_pressed = false;

uint64_t gpio_mask = (1ULL << 2) | (1ULL << 3) | (1ULL << 4);        // this bit-mask enables GPIO 2, 3, 4
                                                                     // for wake up with a button

void setup(void)
{
    /* initialise Serial port */
    Serial.begin(115200);

    /* detect wake up reason */
    ft_report_reboot_reason();

    /* initialise the buttons */
    pinMode(DIAGNOSTICS_BUTTON, INPUT_PULLUP);
    attachInterrupt(DIAGNOSTICS_BUTTON, isr_diagnostics, FALLING);  // RISING edge might be more stable
    pinMode(OTA_BUTTON, INPUT_PULLUP);
    attachInterrupt(OTA_BUTTON, isr_ota, FALLING);
    pinMode(WARNING_BUTTON, INPUT_PULLUP);
    attachInterrupt(WARNING_BUTTON, isr_warning, FALLING);
}

void loop(void)
{
    int i = 10;
    unsigned long line_time;
    volatile unsigned long last_line = 0;
    
    while (i > 0)
    {
        line_time = millis();
        if (line_time - last_line > 1000)
        {
            last_line = line_time;
            Serial.println(" ----");
        }
        if (diagnostics_pressed) 
        {
            diagnostics_pressed = false;
            Serial.println("  ---- Diagnostics Button was pressed");
            i--;
        }
        if (ota_pressed)
        {
            ota_pressed = false;
            Serial.println("  ---- OTA Button was pressed");
            i--;
        }
        if (warning_pressed)
        {
            warning_pressed = false;
            Serial.println("  ---- Warning Button was pressed");
            i--;
        }
    }
    delay (1000);                           // delay to prevent accidental wake up with button debounce
    ft_go_to_sleep(60000);                  // 1 min
}











void IRAM_ATTR isr_diagnostics(void)
{
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_diagnostics > DEBOUNCE_DELAY)
    {
        last_interrupt_diagnostics = interrupt_time;
        diagnostics_pressed = true;
    }
}

void IRAM_ATTR isr_ota(void)
{
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_ota > DEBOUNCE_DELAY)
    {
        last_interrupt_ota = interrupt_time;
        ota_pressed = true;
    }
}

void IRAM_ATTR isr_warning(void)
{
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_warning > DEBOUNCE_DELAY)
    {
        last_interrupt_warning = interrupt_time;
        warning_pressed = true;
    }
}

void  ft_go_to_sleep(uint64_t time_in_millis)
{
    esp_err_t result;
     
    result = esp_deep_sleep_enable_gpio_wakeup(gpio_mask, ESP_GPIO_WAKEUP_GPIO_LOW);  // set GPIOs from the bit-mask as wake-up sources
    if (result != ESP_OK)
        Serial.println("Failed to enable GPIO wakeup");
    Serial.println("\nDEVICE STOP\n");
    esp_sleep_enable_timer_wakeup(time_in_millis * 1000);                             // set timer as the second wake-up source
    esp_deep_sleep_start();                                                           // go to sleep
}


/*
 *  Buttons and timer have the same wake-up reason
 */
void ft_report_reboot_reason(void)
{
    esp_reset_reason_t  reset_reason;

    reset_reason = esp_reset_reason();
    if (reset_reason == ESP_RST_UNKNOWN)
        Serial.println("[BOOTING INFO] Unknown reset reason\n");
    else if (reset_reason == ESP_RST_POWERON)
        Serial.println("[BOOTING INFO] Power on reset\n");           // reason for wake up with pushing the Reset button
    else if (reset_reason == ESP_RST_EXT)
        Serial.println("[BOOTING INFO] External reset\n");
    else if (reset_reason == ESP_RST_SW)
        Serial.println("[BOOTING INFO] Software reset\n");
    else if (reset_reason == ESP_RST_PANIC)
        Serial.println("[BOOTING INFO] CPU Panic reset\n");
    else if (reset_reason == ESP_RST_INT_WDT)
        Serial.println("[BOOTING INFO] Interrupt watchdog reset\n");
    else if (reset_reason == ESP_RST_TASK_WDT)
        Serial.println("[BOOTING INFO] Task watchdog reset\n");
    else if (reset_reason == ESP_RST_WDT)
        Serial.println("[BOOTING INFO] General watchdog reset\n");
    else if (reset_reason == ESP_RST_DEEPSLEEP)                       // reason for wake up with a button or a timer
        Serial.println("[BOOTING INFO] Deep sleep reset\n");
    else if (reset_reason == ESP_RST_BROWNOUT)
        Serial.println("[BOOTING INFO] Brownout reset\n");
    else if (reset_reason == ESP_RST_SDIO)
        Serial.println("[BOOTING INFO] SDIO reset\n");
    else
        Serial.println("[BOOTING INFO] Reset info is missing\n");
}
 
