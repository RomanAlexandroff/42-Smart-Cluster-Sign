/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   42-Smart-Cluster-Sign.h                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 12:50:00 by raleksan          #+#    #+#             */
/*   Updated: 2025/04/11 16:30:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _42_SMART_CLUSTER_SIGN_H
# define _42_SMART_CLUSTER_SIGN_H

# include <Arduino.h>
# include <LittleFS.h>
# include <HTTPClient.h>            // new library for cloud pull OTA update
# include <Update.h>                // new library for cloud pull OTA update
# include <ArduinoJson.h>           // new library for cloud pull OTA update
# include <mbedtls/sha256.h>        // new library for cloud pull OTA update
# ifdef DEBUG
    # include <stdio.h>
# endif
# include <stdint.h>
# include <esp_system.h>
# include <esp_sleep.h>
# include <driver/adc.h>
# include <driver/gpio.h>
# include <esp_task_wdt.h>
# include "bitmap_library.h"
# include "config.h"
# include "globals.h"

/* intra_interaction.cpp */
ERROR_t         fetch_exams(void);

/* battery_management.cpp */
void            battery_check(void);
void            battery_init(void);

/* buttons_handling.cpp */
void            buttons_init(void);
void IRAM_ATTR  isr_diagnostics(void);
void IRAM_ATTR  isr_ota(void);
void IRAM_ATTR  isr_warning(void);

/* cluster_number_mode.cpp */
void            cluster_number_mode(unsigned int* p_sleep_length);

/* display_handling.cpp */
void            draw_colour_bitmap(const unsigned char* black_image, const unsigned char* red_image);
void IRAM_ATTR  display_cluster_number(IMAGE_t mode);
void            clear_display(void);
void IRAM_ATTR  display_init(void);

/* exam_mode.cpp */
void            exam_mode(void);

/* file_system.cpp */
ERROR_t         secret_verification(String text);
void            data_restore(const char* file_name);
void            data_integrity_check(void);
ERROR_t         write_to_file(const char* file_name, char* input);
ERROR_t         read_from_file(const char* file_name, char* output);
ERROR_t         file_sys_init(void);

/* ota.cpp */
void ota_handling(void);

/* other.cpp */
void            go_to_sleep(uint64_t time_in_millis);
void IRAM_ATTR  ft_delay(uint64_t time_in_millis);
void            serial_init(void);
void            wifi_connect(void);
# ifdef EXAM_SIMULATION
    String      exam_simulation(void);
# endif

/* power_down_recovery.cpp */
void            power_down_recovery(void);

/* telegram_bot.cpp */
void            telegram_check(void);

/* telegram_compose_message.cpp */
String          compose_message(int32_t subject, int16_t days_left);

/* time_utilities.cpp */
int16_t         expiration_counter(void);
bool            unix_timestamp_decoder(uint8_t* p_day, uint8_t* p_month, uint16_t* p_year);
int             winter_summer_time_offset(int year, int month, int day, int hour);
bool            get_and_ensure_current_time(String server_response);
unsigned int    time_till_wakeup(void);
unsigned int    time_till_event(int8_t hours, uint8_t minutes);
int             time_sync(unsigned int preexam_time);

/* watchdog.cpp */
void IRAM_ATTR  watchdog_start(void);
void IRAM_ATTR  watchdog_reset(void);
void IRAM_ATTR  watchdog_stop(void);
void            watchdog_init(void);

#endif
 
