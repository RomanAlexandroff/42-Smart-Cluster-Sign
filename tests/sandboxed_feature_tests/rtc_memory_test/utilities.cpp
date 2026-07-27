/*
 *    ESP32 RTC memory retention test.
 *    raleksan, 12.2024
 */

# include "rtc_memory_test.h"

void  ft_serial_init(void)
{
    uint8_t i;
    
    i = 15;
    Serial.begin(BAUD_RATE);
    while (i > 0)
    {
        Serial.printf("-");
        delay(100);
        i--;
    }
    Serial.println("");
}

/*
 *  If it is the first program cycle,
 *  initializes data, allocates memory,
 *  fills the memory with a known pattern
 */
void  ft_memory_init(void)
{
    Serial.println("Initializing RTC memory...");
    rtc_g.bytes_counter = 1;
    rtc_g.content = (uint8_t *)malloc(1);
    if (rtc_g.content != nullptr)
        rtc_g.content[0] = 0xAA;
    else
    {
        Serial.println("Memory allocation failed at the start.");
        ft_stop_program_execution();
    }
}

/*
 *  Checks if the memory content was not
 *  corrupted over the deep sleep and
 *  outputs the results.
 *  Stops program execution if detects
 *  memory loss or corruption.
 */
void  ft_check_memory_content(void)
{
    bool intact = true;
    
    for (uint32_t i = 0; i < rtc_g.bytes_counter - 1; i++)
    {
        if (rtc_g.content[i] != 0xAA)
        {
            intact = false;
            break;
        }
    }
    if (!intact)
    {
        Serial.print("RTC memory overflow detected at ");
        Serial.print(rtc_g.bytes_counter - 1);
        Serial.println(" bytes.");
        ft_stop_program_execution();
    }
    else
    {
        Serial.print("RTC memory intact at ");
        Serial.print(rtc_g.bytes_counter);
        Serial.println(" bytes.");
    }
}

/*
 *  Allocates 1 more byte of memory,
 *  increments the bytes counter.
 *  Stops program execution if detects
 *  memory allocation fail.
 */
void  ft_expand_memory_content(void)
{
    uint8_t *buffer;

    rtc_g.bytes_counter++;
    buffer = (uint8_t *)realloc(rtc_g.content, rtc_g.bytes_counter);
    if (buffer != nullptr)
    {
        rtc_g.content = buffer;
        rtc_g.content[rtc_g.bytes_counter - 1] = 0xAA;
    }
    else
    {
        Serial.println("Memory allocation failed.");
        ft_stop_program_execution();
    }
}

void  ft_stop_program_execution(void)
{
    while (1)
        delay(1000);
}
