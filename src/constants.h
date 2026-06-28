/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   constants.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 13:00:00 by raleksan          #+#    #+#             */
/*   Updated: 2024/12/09 16:30:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/*                                                                            */
/*   The following constants are not expected to be changed at any point.     */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONSTANTS_H
# define CONSTANTS_H

/* General constants */
# define WD_NUMBER_OF_CORES      1                            // watchdog
# define REBOOT                  1000
# define mS_TO_uS_FACTOR         1000ull                      // milliseconds to microseconds
# define mS_TO_S_FACTOR          1000ul                       // milliseconds to seconds
# define MINUTE_MS               60000
# define HOUR_MS                 3600000
# define MONTHS_DAYS             31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
# define YEAR_DAYS               365
# define NOT_FOUND               -1
# define FAILED_TO_COUNT         -128
# define MIN_SLEEP_LIMIT_MS      100ull                       // deep sleep, 0.1 seconds
# define MAX_SLEEP_LIMIT_MS      172800000ull                 // deep sleep, 48 hours

/* Buttons */
# define DIAGNOSTICS_BUTTON      D0
# define OTA_BUTTON              D1
# define WARNING_BUTTON          D2
# define GPIO_MASK               (1ULL << DIAGNOSTICS_BUTTON) | (1ULL << OTA_BUTTON) | (1ULL << WARNING_BUTTON)

/* Display SPI port */
# define BUSY_PIN                D4
# define RST_PIN                 D5
# define DC_PIN                  D6
# define SPI_SS_PIN              D7
# define SPI_SCK_PIN             D8
# define SPI_MISO_PIN            -1
# define SPI_MOSI_PIN            D10

/* Display driver */
# define GxEPD2_DISPLAY_CLASS    GxEPD2_3C
# define GxEPD2_DRIVER_CLASS     GxEPD2_750c_Z08
# define GxEPD2_3C_IS_GxEPD2_3C  true
# define IS_GxEPD2_3C(x)         IS_GxEPD(GxEPD2_3C_IS_, x)
# define MAX_DISPLAY_BUFFER_SIZE 96124ul
# define MAX_HEIGHT(EPD)         (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))

/* Images constants */
typedef enum {
    CLUSTER = 1000,
    DEFAULT_IMG,
    INTRA_ERROR,
    SECRET_EXPIRED,
    EXAM_DAY,
    LOW_BATTERY,
    DEAD_BATTERY,
    OTA_WAITING,
    OTA_SUCCESS,
    OTA_FAIL,
    OTA_CANCELED,
    TELEGRAM_ERROR,
    TELEGRAM_STATUS
} IMAGE_t;

/* Errors constants */
typedef enum {
    UNKNOWN = 2000,
    TIME_OK,
    TIME_NO_WIFI,
    TIME_NO_SERVER,
    TIME_NO_DST,
    INTRA_OK,
    INTRA_NO_SERVER,
    INTRA_NO_TOKEN,
    INTRA_NO_INFO,
    FS_OK,
    FS_INIT_FAIL,
    FS_OPEN_FAIL,
    FS_EMPTY_FILE,
    FS_ENTRY_ERROR,
    FS_VALID_SECRET,
    FS_INVALID_SECRET,
    FS_NOT_A_SECRET
} ERROR_t;

#endif
 
