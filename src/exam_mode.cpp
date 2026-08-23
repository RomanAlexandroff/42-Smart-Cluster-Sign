/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exam_mode.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 13:00:00 by raleksan          #+#    #+#             */
/*   Updated: 2024/11/27 13:40:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42-Smart-Cluster-Sign.h"

static unsigned int exam(void)
{
    unsigned int  exam_remaining_time;

    watchdog_reset();
    exam_remaining_time = time_till_event(com_g.exam_end_hour, com_g.exam_end_minutes);
    draw_on_display(EXAM_ACTIVE);                                                        // execution takes 25 sec
    rtc_g.exam_status = false;
    return (exam_remaining_time);
}


/*
*   Waits out until exam using light sleep.
*   Drawing functions, time_sync and
*   ft_delay have their own watchdog resets.
*/
static void preexam_warning(unsigned int* p_preexam_time)
{
    int minutes;

    minutes = time_sync(*p_preexam_time);
    if (minutes == 60 || minutes == 50)
    {
        draw_on_display(PREEXAM_50);                                                    // execution takes 25 sec
        ft_delay((minutes - 40) * 60000);
        minutes = 40;
    }
    if (minutes == 40 || minutes == 30 || minutes == 20)
    {
        draw_on_display(PREEXAM_25);                                                    // execution takes 25 sec
        ft_delay((minutes - 10) * 60000);
        minutes = 10;
    }
    if (minutes == 10)
    {
        draw_on_display(PREEXAM_5);                                                     // execution takes 25 sec
        ft_delay(480000);
    }
    *p_preexam_time = 0;
}


/*
*   Under normal circumstances the execution
*   time of this function is precisely 1 hour.
*   The function also accounts for situations
*   when it starts executing too early or too
*   late due to any timing errors.
*   
*   Two calls to fetch exams are not redundant.
*/
void  exam_mode(void)
{
    unsigned int  preexam_time;

    watchdog_reset();
    fetch_exams();
    if (!rtc_g.exam_status)
        return ;
    preexam_time = time_till_event(rtc_g.exam_start_hour, rtc_g.exam_start_minutes);
    if (preexam_time > 2 * ONE_HOUR_MS)
        return ;
    if (preexam_time > 10 * ONE_MINUTE_MS)
        preexam_warning(&preexam_time);
    if (preexam_time <= 10 * ONE_MINUTE_MS && preexam_time >= 25 * ONE_SECOND_MS)
        ft_delay(preexam_time - 25 * ONE_SECOND_MS);
    fetch_exams();
    if (!rtc_g.exam_status)
        return ;
    go_to_sleep(exam());
}
 
