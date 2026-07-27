/*
 *    ESP32 RTC memory retention test.
 *    raleksan, 12.2024
 */

#ifndef RTC_MEMORY_TEST_H
# define RTC_MEMORY_TEST_H
 
# include <Arduino.h>
# include <esp_sleep.h>

# define BAUD_RATE       115200
# define SLEEP_TIME_S    5ull
# define S_TO_uS_FACTOR  1000000ull             // seconds to microseconds 

struct s_rtc_data
{
    uint32_t  bytes_counter;
    uint8_t   *content;
};
extern s_rtc_data rtc_g;

void  ft_serial_init(void);
void  ft_memory_init(void);
void  ft_check_memory_content(void);
void  ft_expand_memory_content(void);
void  ft_stop_program_execution(void);

#endif
 
