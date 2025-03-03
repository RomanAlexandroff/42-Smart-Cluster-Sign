/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file_system.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 13:02:00 by raleksan          #+#    #+#             */
/*   Updated: 2024/11/27 13:40:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42-Smart-Cluster-Sign.h"

ERROR_t secret_verification(String input)
{
    char    secret_buffer[74];
    bool    exam_status_buffer;
    ERROR_t intra_result;
    
    watchdog_reset();
    if (input.length() != 73)
        return (FS_NOT_A_SECRET);
    if (input.substring(0, 1) != "s")
        return (FS_NOT_A_SECRET);
    strcpy(secret_buffer, rtc_g.Secret);
    exam_status_buffer = rtc_g.exam_status;
    input.toCharArray(rtc_g.Secret, sizeof(rtc_g.Secret));
    intra_result = fetch_exams();
    strcpy(rtc_g.Secret, secret_buffer);
    rtc_g.exam_status = exam_status_buffer;
    if (intra_result == INTRA_NO_TOKEN)
        return (FS_INVALID_SECRET);
    return (FS_VALID_SECRET);
}

void data_restore(const char* file_name)
{
    if (!file_name)
        return;
    watchdog_reset();
    if (strcmp(file_name, "/secret.txt") == 0)
    {
        if (read_spiffs_file(file_name, rtc_g.Secret) == FS_OK)
            DEBUG_PRINTF("\n[FILE SYSTEM] Successfully restored data from %s.\n", file_name);
        else
            DEBUG_PRINTF("\n[FILE SYSTEM] Failed to restore data from %s!\n", file_name);
        DEBUG_PRINTF("[FILE SYSTEM] The rtc_g.Secret variable value is now:\n%s\n", rtc_g.Secret);
        return;
    }
    else if (strcmp(file_name, "/chat_id.txt") == 0)
    {
        if (read_spiffs_file(file_name, rtc_g.chat_id) == FS_OK)
            DEBUG_PRINTF("\n[FILE SYSTEM] Successfully restored data from %s.\n", file_name);
        else
            DEBUG_PRINTF("\n[FILE SYSTEM] Failed to restore data from %s!\n", file_name);
        DEBUG_PRINTF("[FILE SYSTEM] The rtc_g.chat_id variable value is now:\n%s\n", rtc_g.chat_id);
        return;
    }
}

void  data_integrity_check(void)
{
    watchdog_reset();
    if (!LittleFS.exists("/secret.txt"))
    {
        DEBUG_PRINTF("\n[FILE SYSTEM] The secret.txt file does not exist. Creating...\n");
        char input[] = SECRET;
        if (write_spiffs_file("/secret.txt", input) == FS_OK)
            DEBUG_PRINTF("\n[FILE SYSTEM] secret.txt file has been created.\n");
        else
            DEBUG_PRINTF("\n[FILE SYSTEM] Failed to create the secret.txt file!\n");
    }
    if (!LittleFS.exists("/chat_id.txt"))
    {
        DEBUG_PRINTF("\n[FILE SYSTEM] The chat_id.txt file does not exist. Creating...\n");
        char input[] = "0000000000000";
        if (write_spiffs_file("/chat_id.txt", input) == FS_OK)
        {
            DEBUG_PRINTF("\n[FILE SYSTEM] chat_id.txt file has been created.\n");
            DEBUG_PRINTF("\n[FILE SYSTEM] To set chat_id write \"/status\" into the Telegram chat %s", BOT_NAME);
        }
        else
            DEBUG_PRINTF("\n[FILE SYSTEM] Failed to create the chat_id.txt file!\n");
        display_cluster_number(TELEGRAM_ERROR);
    }
    data_restore("/secret.txt");
    data_restore("/chat_id.txt");
    if (!rtc_g.from_name[0])
    {
        strcpy(rtc_g.from_name, "User");
        DEBUG_PRINTF("\n[FILE SYSTEM] User name has been set to: %s\n", rtc_g.from_name);
    }
}

ERROR_t  write_spiffs_file(const char* file_name, char* input)
{
    File  file;
    short i;

    if (!file_name || !input)
        return FS_ENTRY_ERROR;
    i = 0;
    watchdog_reset();
    while (i < 5)
    {
        file = LittleFS.open(file_name, "w");
        if (file)
            break;
        DEBUG_PRINTF("[FILE SYSTEM] An error occurred while opening %s file for writing in LittleFS. Retrying.\n", file_name);
        i++;
        delay(100);
    }
    if (!file)
    {
        DEBUG_PRINTF("[FILE SYSTEM] Failed to open %s file for writing in LittleFS. Its dependant function will be unavailable during this programm cycle.\n", file_name);
        return FS_OPEN_FAIL;
    }
    file.println(input);
    file.close();
    return FS_OK;
}

ERROR_t  read_spiffs_file(const char* file_name, char* output)
{
    File    file;
    short   i;
    String  buffer;

    if (!file_name)
        return FS_ENTRY_ERROR;
    i = 0;
    watchdog_reset();
    while (i < 5)
    {
        file = LittleFS.open(file_name, "r");
        if (file)
            break;
        DEBUG_PRINTF("[FILE SYSTEM] An error occurred while opening %s file for reading in LittleFS. Retrying.\n", file_name);
        i++;
        delay(100);
    }
    if (!file)
    {
        DEBUG_PRINTF("[FILE SYSTEM] Failed to open %s file for reading in LittleFS. Its dependant function will be unavailable during this programm cycle.\n", file_name);
        return FS_OPEN_FAIL;
    }  
    buffer = file.readStringUntil('\n');
    file.close();
    buffer.trim();
    if (buffer.length() == 0)
        return FS_EMPTY_FILE;
    strcpy(output, buffer.c_str());
    return FS_OK;
}

ERROR_t  spiffs_init(void)
{
    short i;

    i = 0;
    watchdog_reset();
    if (!LittleFS.begin(true) && i < 5)
    {
        DEBUG_PRINTF("\n[FILE SYSTEM] Failed to initialise SPIFFS. Retrying...\n");
        ft_delay(500);
        i++;
    }
    else
    {
        DEBUG_PRINTF("\n[FILE SYSTEM] SPIFFS is successfully initialised.\n");
        return FS_OK;
    }
    DEBUG_PRINTF("\n[FILE SYSTEM] SPIFFS was not initialised. Reading and Writing data is unavailable this session.\n");
    return FS_INIT_FAIL;
}
 
