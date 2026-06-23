/* ************************************************************************** */
/*                                                                            */
/*   42 Smart Cluster Sign                                :::      ::::::::   */
/*   src.ino                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 12:50:00 by raleksan          #+#    #+#             */
/*   Updated: 2026/06/20 17:20:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42-Smart-Cluster-Sign.h"

void  setup(void)
{
    watchdog_init();
    display_init();
    #ifdef DEBUG
        serial_init();
    #endif
    file_sys_init();
    buttons_init();
    battery_init();
    power_down_recovery();
    battery_check();
    telegram_check();
}

/*
 *  Despite the name the function runs only once.
 *  The name has to respect the Arduino IDE API.
 */
void  loop(void)
{
    unsigned int  sleep_length;

    watchdog_reset();
    if (rtc_g.exam_status)
        exam_mode();
    cluster_number_mode(&sleep_length);
    ota_handling();                         // needs to be here to have access to time values
    go_to_sleep(sleep_length);
    DEBUG_PRINTF("  ---- This message will never be printed out");
}
 
