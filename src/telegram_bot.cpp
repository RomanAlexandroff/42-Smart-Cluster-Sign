/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   telegram_bot.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 13:02:56 by raleksan          #+#    #+#             */
/*   Updated: 2024/11/27 14:00:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/*                                                                            */
/*   These functions are for checking new Telegram messages, reading them     */
/*   and reacting to them.                                                    */
/*   WARNING! DO NOT CALL go_to_sleep(), ft_delay() or ESP.restart()          */
/*   FROM THESE FUNCTIONS! THE DEVICE WILL BECOME UNRESPONSIVE TO ANY         */
/*   MESSAGES FROM THE TELEGRAM CHAT! YOU MAY DO IT ONLY WHEN YOU ARE ABOUT   */
/*   TO EXIT FROM telegram_check().                                           */
/*                                                                            */
/* ************************************************************************** */

#include "42-Smart-Cluster-Sign.h"

static void reply_machine(String text)
{
    String  message;
    ERROR_t check_result;

    if (text == "/status")
    {
        bot.sendMessage(String(rtc_g.chat_id), compose_message(TELEGRAM_STATUS, 0), "");
        return;
    }
    else if (text == "/ota")
    {
        com_g.ota = true;
        return;
    }
    else
    {
        text.remove(0, 1);
        check_result = secret_verification(text);
        if (check_result == FS_VALID_SECRET)
        {
            text.toCharArray(rtc_g.Secret, sizeof(rtc_g.Secret));
            write_spiffs_file("/secret.txt", rtc_g.Secret);
            message = "Accepted!\nThe SECRET token has been renewed.\n\n";
            message += "Current token now is:\n" + String(rtc_g.Secret);
            bot.sendMessage(String(rtc_g.chat_id), message, "");
            return;
        }
        else if (check_result == FS_INVALID_SECRET)
        {
            message = "It looks like a SECRET token, but Intra rejects it. ";
            message += "It is probably an old token. I cannot use it.";
            bot.sendMessage(String(rtc_g.chat_id), message, "");
            return;
        }
        else
        {
            message = "I am sorry, but I do not understand \"";
            message += text + "\".\n";
            message += "You may try to use \"/status\" command";
            bot.sendMessage(String(rtc_g.chat_id), message, "");
        }
    }
}

static void  sender_handling(uint8_t i)
{
    String  id_buffer;
    String  name_buffer;
    String  message;

    id_buffer = bot.messages[i].chat_id;
    if (id_buffer != String(rtc_g.chat_id))
    {
        message = "I have been messaged by another User:";
        message += "\n  - name: " + bot.messages[i].from_name;
        message += "\n  - chat ID: " + bot.messages[i].chat_id;
        message += "\n  - text: " + bot.messages[i].text;
        bot.sendMessage(String(rtc_g.chat_id), message, "");
    }
    id_buffer.toCharArray(rtc_g.chat_id, sizeof(rtc_g.chat_id));
    write_spiffs_file("/chat_id.txt", rtc_g.chat_id);
    name_buffer = bot.messages[i].from_name;
    name_buffer.toCharArray(rtc_g.from_name, sizeof(rtc_g.from_name));
}

static void  new_messages(short message_count)
{
    uint8_t i;
    String  text;

    i = 0;
    DEBUG_PRINTF("\n[TELEGRAM BOT] Handling new Telegram messages\n");
    DEBUG_PRINTF("[TELEGRAM BOT] Number of messages to handle: %d\n", message_count);
    while (i < message_count) 
    {
        watchdog_reset();
        DEBUG_PRINTF("[TELEGRAM BOT] Handling loop iterations: i = %d\n", i);
        sender_handling(i);
        text = bot.messages[i].text;
        DEBUG_PRINTF("[TELEGRAM BOT] %s says: ", rtc_g.from_name);
        DEBUG_PRINTF("%s\n\n", text.c_str());
        reply_machine(text);
        i++;
    }
}

void  telegram_check(void)
{
    short message_count;

    if (WiFi.status() != WL_CONNECTED)
        wifi_connect();  
    watchdog_reset();
    message_count = bot.getUpdates(bot.last_message_received + 1);
    while (message_count)
    {
        watchdog_reset();
        new_messages(message_count);
        message_count = bot.getUpdates(bot.last_message_received + 1);
    }
}
 
