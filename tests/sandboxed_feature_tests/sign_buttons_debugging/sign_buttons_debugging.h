
# include <stdio.h>
# include <esp_system.h>
# include <esp_sleep.h>
# include <driver/gpio.h>

# define DIAGNOSTICS_BUTTON      D0
# define OTA_BUTTON              D1
# define WARNING_BUTTON          D2

void IRAM_ATTR        isr_diagnostics(void);
void IRAM_ATTR        isr_ota(void);
void IRAM_ATTR        isr_warning(void);
void                  ft_report_reboot_reason(void);
void                  ft_go_to_sleep(uint64_t time_in_millis);
