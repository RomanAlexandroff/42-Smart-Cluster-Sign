/*
 *    ESP32 RTC memory retention test.
 *    raleksan, 12.2024
 */

The test helps determine the maximum size of the RTC memory by detecting when the expandable
variable called "content" starts losing its values.


=======================
Step-by-Step Execution:
=======================

  1.Initialization:
      The setup() function is called when the ESP32 starts or wakes up from deep sleep.
      The serial communication is initialized to allow printing messages to the serial monitor.
      A delay ensures the serial communication is ready.
  
  2.First Run Check:
      The program checks if the RTC memory has been initialized by examining the bytes counter variable.
      If the counter is 0, it means that this is the first loop cycle of the program:
        - The counter is set to 1.
        - Memory for one byte is allocated using malloc(1).
        - The allocated memory is filled with a known pattern "0xAA" (in binary "10101010" - an alternating bit pattern).

  3.Subsequent Runs:
      If the bytes counter is not 0, it means that the program has run before and the RTC memory has already been initialized.
      The program checks if the content of the memory is intact by iterating through the allocated memory and verifying that each byte is still 0xAA.
      If any byte is corrupted (not equal to 0xAA), it indicates an RTC memory overflow:
        - The program prints the size at which the overflow occurred.
        - The program enters an infinite loop to stop further execution.
      If the content of the memory is intact:
        - The program prints the current size of the expandable variable.

  4.Memory Expansion:
      The bytes counter is incremented by 1.
      The memory is reallocated to accommodate the new size using realloc().
      If the reallocation is successful:
        - The new byte is set to 0xAA to keep all the memory being tested with a known pattern.
      If the reallocation fails:
        - The program prints an error message.
        - The program enters an infinite loop to stop further execution.

  5.Print Current State:
      The program prints the current number of bytes allocated in the expandable variable.

  6.Deep Sleep:
      The program configures the ESP32 to wake up after SLEEP_TIME_S seconds using esp_sleep_enable_timer_wakeup().
      The ESP32 enters deep sleep mode with esp_deep_sleep_start().

  7.Wake Up:
      After SLEEP_TIME_S seconds, the ESP32 wakes up from deep sleep.
      The setup() function is called again, and the process repeats from step 2.

  SUMMARY:
    The program initializes the RTC memory on the first run.
    On subsequent runs, it checks if the expandable variable is intact.
    It increments the counter and reallocates memory to expand the variable.
    It prints the current state and enters deep sleep for a short time.
    The process repeats until an RTC memory overflow is detected, at which point the program stops further execution.

 
