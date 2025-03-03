/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   globals.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 13:00:00 by raleksan          #+#    #+#             */
/*   Updated: 2025/03/03 16:00:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "globals.h"

GxEPD2_3C<GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT> display(GxEPD2_750c_Z08(SPI_SS_PIN, DC_PIN, RST_PIN, BUSY_PIN));
WiFiClientSecure Telegram_client;
WiFiClientSecure Intra_client;
UniversalTelegramBot bot(BOT_TOKEN, Telegram_client);

RTC_DATA_ATTR struct rtc_global_variables rtc_g;

struct common_global_variables com_g = {
    .last_diagnostics = 0,
    .last_ota = 0,
    .last_warning = 0,
    .ota = false,
    .exam_end_hour = 0,
    .exam_end_minutes = 0,
    .daylight_flag = false,
    .hour = 0,
    .minute = 0,
    .day = 0,
    .month = 0,
    .year = 0,
};
 
