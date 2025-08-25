/* ************************************************************************** */
/*                                                                            */
/*   42 Smart Cluster Sign                                :::      ::::::::   */
/*   src.ino                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 12:50:00 by raleksan          #+#    #+#             */
/*   Updated: 2025/04/11 13:20:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42-Smart-Cluster-Sign.h"

static void pathfinder(void);

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
    ota_init();
}

void  loop(void)
{
    ota_waiting_loop();
    pathfinder();
}

static void  pathfinder(void)
{
    unsigned int  sleep_length;

    watchdog_reset();
    if (rtc_g.exam_status)
        exam_mode();
    cluster_number_mode(&sleep_length);
    go_to_sleep(sleep_length);
    DEBUG_PRINTF("  ---- This message will never be printed out");
}
 
