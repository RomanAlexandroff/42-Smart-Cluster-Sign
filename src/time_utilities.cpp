/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_utilities.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 13:00:00 by raleksan          #+#    #+#             */
/*   Updated: 2024/12/09 17:20:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42-Smart-Cluster-Sign.h"

bool unix_timestamp_decoder(uint8_t* p_day, uint8_t* p_month, uint16_t* p_year)
{
    const uint32_t SECONDS_PER_DAY        = 86400UL;
    const uint32_t DAYS_OFFSET_1970_TO_CE = 719468UL;
    const uint32_t DAYS_PER_400_YEARS     = 146097UL;
    const uint32_t DAYS_PER_100_YEARS     = 36524UL;
    const uint32_t DAYS_PER_4_YEARS       = 1461UL;
    const uint32_t DAYS_PER_YEAR          = 365UL;
    const uint32_t DAYS_TO_MONTH_FACTOR   = 153UL;
    const uint32_t MONTH_FACTOR           = 5UL;
    const uint32_t MONTH_OFFSET           = 2UL;
    const uint32_t MARCH_BASED_OFFSET     = 3UL;
    const uint32_t JAN_FEB_OFFSET         = 9UL;
    uint32_t       days_since_epoch;
    int32_t        adjusted_days;
    int32_t        era;
    uint32_t       day_of_era;
    uint32_t       year_of_era;
    uint32_t       day_of_year;
    uint32_t       month_param;
    uint32_t       day;
    uint32_t       month;
    uint32_t       year;

    if (rtc_g.secret_expiration < 1000000000UL)
        return (false);
    watchdog_reset();
    days_since_epoch = rtc_g.secret_expiration / SECONDS_PER_DAY;
    adjusted_days = days_since_epoch + DAYS_OFFSET_1970_TO_CE;
    if (adjusted_days >= 0)
        era = adjusted_days / DAYS_PER_400_YEARS;
    else
        era = (adjusted_days - (DAYS_PER_400_YEARS - 1)) / DAYS_PER_400_YEARS;
    day_of_era = adjusted_days - era * DAYS_PER_400_YEARS;
    year_of_era = (day_of_era - day_of_era / (DAYS_PER_4_YEARS) + day_of_era
        / (DAYS_PER_100_YEARS) - day_of_era
        / (DAYS_PER_400_YEARS)) / DAYS_PER_YEAR;
    year = year_of_era + era * 400;
    day_of_year = day_of_era - (DAYS_PER_YEAR * year_of_era + year_of_era / 4 - year_of_era / 100);
    month_param = (MONTH_FACTOR * day_of_year + MONTH_OFFSET) / DAYS_TO_MONTH_FACTOR;
    day = day_of_year - (DAYS_TO_MONTH_FACTOR * month_param + MONTH_OFFSET) / MONTH_FACTOR + 1;
    if (month_param < 10)
        month = month_param + MARCH_BASED_OFFSET;
    else
        month = month_param - JAN_FEB_OFFSET;
    if (month <= 2)
        year += 1;
    *p_day   = (uint8_t)day;
    *p_month = (uint8_t)month;
    *p_year  = (uint16_t)year;
    return (true);
}

int16_t  expiration_counter(void)
{
    const int months_days[12] = {MONTHS_DAYS};
    int       current_month = com_g.month;
    int       days_in_between = 0;
    uint8_t   expire_day;
    uint8_t   expire_month;
    uint16_t  expire_year;
    uint16_t  year_correction;

    watchdog_reset();
    if (!unix_timestamp_decoder(&expire_day, &expire_month, &expire_year))
        return (FAILED_TO_COUNT);
    if (expire_year < com_g.year)
        return (FAILED_TO_COUNT);
    year_correction = (expire_year - com_g.year) * YEAR_DAYS;
    if (expire_month == com_g.month)
        return (expire_day + year_correction - com_g.day);
    else
    {
        while (current_month != expire_month - 1)
        {
            days_in_between += months_days[current_month - 1];
            current_month++;
            if (current_month > 12)
                current_month = 1;
        }
        return (expire_day + year_correction + days_in_between + (months_days[com_g.month - 1] - com_g.day));
    }
}

/*
*   Helper function for winter_summer_time_offset().
*   Uses Zeller's congruence for calculating weekdays.
*/
static int is_weekday(int year, int month, int day)
{
    int k;          // year of the century
    int j;          // century
    int h;          // weekday result

    if (month < 3)
    {
        month += 12;
        year -= 1;
    }
    k = year % 100;
    j = year / 100;
    h = (day + 13 * (month + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
    return ((h + 6) % 7);
}

/*
*   Helper function for winter_summer_time_offset()
*/
static int last_sunday(int year, int month)
{
    int weekday;
    int last_day = 31;

    if (month == 4 || month == 6 || month == 9 || month == 11)
        last_day = 30;
    else if (month == 2)
        last_day = 28;
    weekday = is_weekday(year, month, last_day);
    return (last_day - weekday);
}

int winter_summer_time_offset(int year, int month, int day, int hour)
{
    int start;
    int end;

    start = last_sunday(year, 3);   // March
    end = last_sunday(year, 10);    // October
    if (month < 3 || month > 10)
        return (0);
    if (month > 3 && month < 10)
        return (1);
    if (month == 3)
    {
        if (day > start)
            return (1);
        if (day < start)
            return (0);
        return (hour >= 1);
    }
    if (month == 10)
    {
        if (day < end)
            return 1;
        if (day > end)
            return 0;
        return (hour < 1);
    }
    return (0);
}

/*
*   This function extracts day of the month, month name,
*   year, hours, minutes and seconds from an Intra server
*   response. Then it converts the month name into month
*   number, rounds seconds up into minutes, adjusts hours
*   to the Time Zone and winter/summer time. The function
*   handles overflows and underflows of minutes, hours,
*   days and months. The final result gets outputed into
*   the Serial monitor. The function checks the final
*   result for correctness. The function returns False
*   in case anything of the above goes wrong.
*/
bool get_and_ensure_current_time(const String& server_response)
{
    int     i;
    int     seconds;
    String  current_month;
    int     month_days[12] = {MONTHS_DAYS};
    String  month_names[13] = {
        "-", "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
    };

    watchdog_reset();
    i = server_response.indexOf("Date:");
    if (i == NOT_FOUND)
    {
        i = server_response.indexOf("date:");
        if (i == NOT_FOUND)
        {
            DEBUG_PRINTF("[SYSTEM TIME] Current time was not found in the server response\n\n");
            return (false);
        }
    }
    com_g.day = server_response.substring(i + 11, i + 13).toInt();
    current_month = server_response.substring(i + 14, i + 17);
    com_g.month = 1;
    while (com_g.month <= 12 && month_names[com_g.month] != current_month)
        com_g.month++;
    com_g.year = server_response.substring(i + 18, i + 22).toInt();
    com_g.hour = server_response.substring(i + 23, i + 25).toInt();
    com_g.minute = server_response.substring(i + 26, i + 28).toInt();
    seconds = server_response.substring(i + 29, i + 31).toInt();
    if (seconds > 25)
        com_g.minute += 1;
    if (com_g.minute == 60)
    {
        com_g.minute = 0;
        com_g.hour += 1;
    }
    com_g.hour += TIME_ZONE + winter_summer_time_offset(com_g.year, com_g.month, com_g.day, com_g.hour);
    if (com_g.hour >= 24)
    {
        com_g.hour -= 24;
        com_g.day += 1;
        if (com_g.day > month_days[com_g.month - 1])
        {
            com_g.day = 1;
            com_g.month += 1;

            if (com_g.month > 12)
            {
                com_g.month = 1;
                com_g.year += 1;
            }
        }
    }
    else if (com_g.hour < 0)
    {
        com_g.hour += 24;
        com_g.day -= 1;
        if (com_g.day < 1)
        {
            com_g.month -= 1;
            if (com_g.month < 1)
            {
                com_g.month = 12;
                com_g.year += 1;
            }
            com_g.day = month_days[com_g.month - 1];
        }
    }
    DEBUG_PRINTF("\n[SYSTEM TIME] Obtained time from the Intra server as follows:\n");
    DEBUG_PRINTF("  --hour:   %d\n", com_g.hour);
    DEBUG_PRINTF("  --minute: %d\n", com_g.minute);
    DEBUG_PRINTF("  --day:    %d\n", com_g.day);
    DEBUG_PRINTF("  --month:  %d\n", com_g.month);
    DEBUG_PRINTF("  --year:   %d\n\n", com_g.year);
    if (com_g.day < 1 || com_g.day > 31
      || com_g.year < 2000 || com_g.year > 9999
      || com_g.hour < 0 || com_g.hour > 23
      || com_g.minute < 0 || com_g.minute > 59
      || com_g.month < 1 || com_g.month > 12)
    {
        DEBUG_PRINTF("[SYSTEM TIME] Current time has incorrect values!\n\n");
        return (false);
    }
    return (true);
}


unsigned int  time_till_wakeup(void)
{
    const uint8_t wakeup_hour[] = {WAKE_UP_HOURS};
    uint8_t       i;

    watchdog_reset();
    i = sizeof(wakeup_hour) / sizeof(wakeup_hour[0]) - 1;
    if (com_g.hour >= wakeup_hour[i])
        return ((wakeup_hour[0] + 24 - com_g.hour) * HOUR_MS - (com_g.minute * MINUTE_MS) - millis());
    i = 0;
    while ((wakeup_hour[i] - com_g.hour) <= 0)
        i++;
    return ((wakeup_hour[i] - com_g.hour) * HOUR_MS - (com_g.minute * MINUTE_MS) - millis());
}


/*
*   Returns value in milliseconds.
*   Works on the assumption that
*   the event never crosses midnight
*/
unsigned int  time_till_event(int8_t hours, uint8_t minutes)
{
    long    time_left_ms;

    watchdog_reset();
    time_left_ms = (hours - com_g.hour) * HOUR_MS;
    time_left_ms += (minutes * MINUTE_MS) - (com_g.minute * MINUTE_MS);
    if (time_left_ms < 0)
        time_left_ms = 0;
    return ((unsigned int)time_left_ms);
}


int  time_sync(unsigned int preexam_time)
{
    int minutes;

    watchdog_stop();
    DEBUG_PRINTF("\n[TIME_SYNC] Synchronizing time...\n");
    minutes = ceil(preexam_time / 1000);
    while (minutes % 10 != 0)
    {
        preexam_time = preexam_time - 1000;
        minutes = ceil(preexam_time / 1000);
        ft_delay(1000);
    }
    minutes = ceil(preexam_time / MINUTE_MS);
    while (minutes % 10 != 0 || minutes > 60)
    {
        ft_delay(59990);
        minutes--;
    }
    watchdog_start();
    DEBUG_PRINTF("[TIME_SYNC] Synchronization is complete.\n");
    return (minutes);
}
 
