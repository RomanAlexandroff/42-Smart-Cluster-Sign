/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/09 16:20:00 by raleksan          #+#    #+#             */
/*   Updated: 2024/12/18 13:00:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/*                                                                            */
/*   The following values are adjustable for tuning the software behavior.    */
/*   You may also find some configurations in credentials.h                   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_H
# define CONFIG_H

# define SOFTWARE_VERSION       4.32
# define DEVICE_NAME            "42 Prague Smart Sign"

# define DEBUG                                               // comment out this line to turn off Serial output
# ifdef DEBUG
    #define DEBUG_PRINTF(...)   Serial.printf(__VA_ARGS__)
    #define WD_RESET_INFO       true
# else
    #define DEBUG_PRINTF(...)
    #define WD_RESET_INFO       false
# endif

//# define EXAM_SIMULATION                                   // uncomment to simulate an exam. See exam_simulation() function

# pragma GCC optimize           ("O0")                       // project compilation optimization level
# define BAUD_RATE              115200                       // speed of the Serial communication
# define WAKE_UP_HOURS          6, 9, 12, 15, 18, 21         // e.g. 15 means 15:00
# define RETRIES_LIMIT          3                            // for getting time and exam info, file system operations
# define TIME_ZONE              1                            // campus time zone according to GMT or UTC standart;
                                                                // Make sure to use only the Winter-time time zone; 
                                                                // Include "-" sign if applies. Do not include "+" sign.
/* Time limits */
# define CONNECT_TIMEOUT_S      5                            // wi-fi
# define DEBOUNCE_DELAY_MS      1000ul                       // buttons
# define WD_TIMEOUT_MS          8000                         // watchdog
# define OTA_WAIT_LIMIT_S       600                          // 10 minutes
# define SERVER_WAIT_MS         1000                         // Intra
# define SUBS_CHECK_LIMIT_MS    3900000ul                    // 1 hour 5 minutes
# define DEAD_BATTERY_SLEEP_MS  86400000ull                  // 24 hours

/* Battery config */
# define BATTERY_CRITICAL       400
# define BATTERY_GOOD           750
# define BATTERY_SAMPLES_LIMIT  5

#endif
 
