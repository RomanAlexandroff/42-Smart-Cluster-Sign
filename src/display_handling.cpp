/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display_handling.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: raleksan <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/09 13:00:00 by raleksan          #+#    #+#             */
/*   Updated: 2024/11/27 18:30:00 by raleksan         ###   ########.fr       */
/*                                                                            */
/*                                                                            */
/* ************************************************************************** */

#include "42-Smart-Cluster-Sign.h"


/*
*   For all the text notes on the right side
*   (480x170 window) of the cluster number.
*   Partial display update, text only, no images
*/
static void  draw_text(String output, uint16_t x, uint16_t y)
{
    const int16_t   text_box_x PROGMEM = 0;
    const int16_t   text_box_y PROGMEM = 633;
    const uint16_t  text_width PROGMEM = 480;
    const uint16_t  text_height PROGMEM = 170;

    watchdog_stop();
    display.setRotation(3);
    display.setFont(&FreeSansBold24pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setPartialWindow(text_box_x, text_box_y, text_width, text_height);
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y);
        display.print(output);
    }
    while (display.nextPage());
    watchdog_start();
}


/*
*   Only for the text note about the exam start
*   time on the right side (480x170 window) of
*   the cluster number.
*   Partial display update, text only, no images
*/
static void  draw_exam_start_time(void)
{
    String         text;
    const int16_t  text_x PROGMEM = 27;
    const int16_t  text_y PROGMEM = 776;
    const int16_t  window_x PROGMEM = 0;
    const int16_t  window_y PROGMEM = 740;
    const int16_t  window_width PROGMEM = 480;
    const int16_t  window_height PROGMEM = 40;

    watchdog_stop();
    text = "TODAY AT ";
    text += String(rtc_g.exam_start_hour);
    if (rtc_g.exam_start_minutes < 10)
        text += ":0" + String(rtc_g.exam_start_minutes);
    else
        text += ":" + String(rtc_g.exam_start_minutes);
    DEBUG_PRINTF("[THE DISPLAY] Printing the exam time: %s\n", text.c_str());
    display.setFont(&FreeSansBold24pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setRotation(3);
    display.setPartialWindow(window_x, window_y, window_width, window_height);
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(text_x, text_y);
        display.print(text);
    }
    while (display.nextPage());
    watchdog_start();
}


/*
*   Only for the right-side (480x170) images
*   Partial display update, Black and white only
*/
static void draw_bitmap_partial_update(const unsigned char* image, uint16_t width, uint16_t height)
{
    watchdog_stop();
    display.setRotation(0);
    display.setPartialWindow(630, 0, width, height);
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(630, 0, image, width, height, GxEPD_BLACK);
    }
    while (display.nextPage());
    watchdog_start();
}


/*
*   For full-screen b/r/w images.
*   Full display update, Black-Red-White only
*/
void  draw_colour_bitmap(const unsigned char* black_image, const unsigned char* red_image)
{
    watchdog_stop();
    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_BLACK);
        display.drawBitmap(0, 0, black_image, 800, 480, GxEPD_WHITE);
        display.drawBitmap(0, 0, red_image, 800, 480, GxEPD_RED);
    }
    while (display.nextPage());
    watchdog_start();
}


/*
*   For any size b/w images.
*   Full display update, Black-White only
*/
static void draw_bitmap_full_update(const unsigned char* image, uint16_t width, uint16_t height)
{
    watchdog_stop();
    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(0, 0, image, width, height, GxEPD_BLACK);
    }
    while (display.nextPage());
    watchdog_start();
}


/*
*   This function displays the number of the cluster and
*   any additional image/message separately.
*   The function ensures that whatever is already drawn
*   on the display does not get drawn repeatedly.
*   
*   "displaying_now = CLUSTER" is there to unblock drawing
*   of ALL the additional images/messages. Useful after exams.
*/
void  display_cluster_number(IMAGE_t mode)
{
    RTC_DATA_ATTR static bool    display_cluster;
    RTC_DATA_ATTR static IMAGE_t displaying_now;

    watchdog_reset();
    if (display_cluster && mode == displaying_now)
    {
        DEBUG_PRINTF("\n[THE DISPLAY] Nothing new to draw. Drawing aborted\n\n");
        return;
    }
    if (!display_cluster)
    {
        DEBUG_PRINTF("\n[THE DISPLAY] Drawing the cluster number with...\n");
        draw_bitmap_full_update(cluster_number_img, 630, 480);
        display_cluster = true;
        displaying_now = CLUSTER;
    }
    if (mode == DEFAULT_IMG && (displaying_now != DEFAULT_IMG && displaying_now != LOW_BATTERY))
    {
        DEBUG_PRINTF("[THE DISPLAY] ...the default cluster icons\n");
        draw_bitmap_partial_update(default_cluster_icons, 170, 480);
        displaying_now = DEFAULT_IMG;
    }
    else if (mode == INTRA_ERROR && displaying_now != INTRA_ERROR)
    {
        DEBUG_PRINTF("[THE DISPLAY] ...the Intra error warning\n");
        draw_bitmap_partial_update(intra_error_img, 170, 480);
        displaying_now = INTRA_ERROR;
    }
    else if (mode == SECRET_EXPIRED && displaying_now != SECRET_EXPIRED)
    {
        DEBUG_PRINTF("[THE DISPLAY] ...the Secret expiration warning\n");
        draw_bitmap_partial_update(secret_expire_img, 170, 480);
        displaying_now = SECRET_EXPIRED;
    }
    else if (mode == EXAM_DAY && displaying_now != EXAM_DAY)
    {
        DEBUG_PRINTF("[THE DISPLAY] ...the exam time note\n");
        draw_bitmap_partial_update(reserve_note_img, 170, 480);
        delay (7000);                                                 // experimentally derived
        draw_exam_start_time();
        display_cluster = false;
        displaying_now = EXAM_DAY;
    }
    else if (mode == LOW_BATTERY && displaying_now != LOW_BATTERY)
    {
        DEBUG_PRINTF("[THE DISPLAY] ...the low battery warning\n");
        draw_bitmap_partial_update(low_battery_img, 170, 480);
        displaying_now = LOW_BATTERY;
    }
    else if (mode == OTA_WAITING && displaying_now != OTA_WAITING)
    {
        DEBUG_PRINTF("[THE DISPLAY] ...the OTA notification\n");
        draw_text("   WAITING FOR\n   OTA UPDATE", 0, 710);
        displaying_now = OTA_WAITING;
    }
    else if (mode == OTA_SUCCESS && displaying_now != OTA_SUCCESS)
    {
        DEBUG_PRINTF("[THE DISPLAY] ...the OTA notification\n");
        draw_text("   OTA UPDATE\n   SUCCESS", 0, 710);
        displaying_now = OTA_SUCCESS;
    }
    else if (mode == OTA_FAIL && displaying_now != OTA_FAIL)
    {
        DEBUG_PRINTF("[THE DISPLAY] ...the OTA notification\n");
        draw_text("   OTA UPDATE\n   FAIL", 0, 710);
        displaying_now = OTA_FAIL;
    }
    else if (mode == OTA_CANCELED && displaying_now != OTA_CANCELED)
    {
        DEBUG_PRINTF("[THE DISPLAY] ...the OTA notification\n");
        draw_text("   OTA UPDATE\n   WAS CANCELED", 0, 710);
        displaying_now = OTA_CANCELED;
    }
    else if (mode == TELEGRAM_ERROR && displaying_now != TELEGRAM_ERROR)
    {
        DEBUG_PRINTF("[THE DISPLAY] ...the Telegram error warning\n");
        draw_text("   TELEGRAM BOT\n   ERROR", 0, 710);
        displaying_now = TELEGRAM_ERROR;
    } 
    DEBUG_PRINTF("[THE DISPLAY] The drawing process is complete\n");
}

void  clear_display(void)
{
    watchdog_reset();
    display.clearScreen();
    display.writeScreenBuffer();
}

void  display_init(void)
{
    watchdog_reset();
    SPI.end();
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_SS_PIN);
    display.init(115200);
}
 
