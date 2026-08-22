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
#include "display_handling.h"

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
static void  draw_exam_start_time(uint16_t hour, uint16_t minutes)
{
    String         text;
    const int16_t  text_x PROGMEM = 27;
    const int16_t  text_y PROGMEM = 776;
    const int16_t  window_x PROGMEM = 0;
    const int16_t  window_y PROGMEM = 740;
    const int16_t  window_width PROGMEM = 480;
    const int16_t  window_height PROGMEM = 40;

    watchdog_stop();
    display.init(115200);                             // insures the display is still on
    text = "TODAY AT ";
    text += String(hour);
    if (minutes < 10)
        text += ":0" + String(minutes);
    else
        text += ":" + String(minutes);
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
static void  draw_colour_bitmap_raw(const unsigned char* black_image, const unsigned char* red_image)
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
*   Decides what to draw on the display in Exam mode.
*/
static void draw_exam_screen(EXAM_SCREEN_t screen)
{
    if (screen == EXAM_SCREEN_PRE_50)
        draw_colour_bitmap_raw(preexam_50mins, preexam_warning_red);
    else if (screen == EXAM_SCREEN_PRE_25)
        draw_colour_bitmap_raw(preexam_25mins, preexam_warning_red);
    else if (screen == EXAM_SCREEN_PRE_5)
        draw_colour_bitmap_raw(preexam_5mins, preexam_warning_red);
    else if (screen == EXAM_SCREEN_ACTIVE)
        draw_colour_bitmap_raw(exam_warning_black, exam_warning_red);
}


/*
*   In Cluster-number mode there is a strip of display on
*   the right where some useful information gets displayed.
*   This function decides what to draw on that strip.
*/
static void draw_cluster_side(const DISPLAY_STATE_t& state)
{
    if (state.cluster_side == CLUSTER_SIDE_DEFAULT)
        draw_bitmap_partial_update(default_cluster_icons, 170, 480);
    else if (state.cluster_side == CLUSTER_SIDE_INTRA_ERROR)
        draw_bitmap_partial_update(intra_error_img, 170, 480);
    else if (state.cluster_side == CLUSTER_SIDE_SECRET_EXPIRED)
        draw_bitmap_partial_update(secret_expire_img, 170, 480);
    else if (state.cluster_side == CLUSTER_SIDE_EXAM_DAY)
    {
        draw_bitmap_partial_update(reserve_note_img, 170, 480);
        draw_exam_start_time(state.exam_hour, state.exam_minutes);
    }
    else if (state.cluster_side == CLUSTER_SIDE_LOW_BATTERY)
        draw_bitmap_partial_update(low_battery_img, 170, 480);
    else if (state.cluster_side == CLUSTER_SIDE_OTA_WAITING)
        draw_text("   WAITING FOR\n   OTA UPDATE", 0, 710);
    else if (state.cluster_side == CLUSTER_SIDE_OTA_SUCCESS)
        draw_text("   OTA UPDATE\n   SUCCESS", 0, 710);
    else if (state.cluster_side == CLUSTER_SIDE_OTA_FAIL)
        draw_text("   OTA UPDATE\n   FAIL", 0, 710);
    else if (state.cluster_side == CLUSTER_SIDE_OTA_CANCELED)
        draw_text("   OTA UPDATE\n   WAS CANCELED", 0, 710);
    else if (state.cluster_side == CLUSTER_SIDE_TELEGRAM_ERROR)
        draw_text("   TELEGRAM BOT\n   ERROR", 0, 710);
}


/*
*   Compares the two given display states.
*   Returns true if they are completely equal.
*   Otherwise returns false.
*/
static bool same_display_state(const DISPLAY_STATE_t& a, const DISPLAY_STATE_t& b)
{
    return (
        a.view == b.view &&
        a.cluster_side == b.cluster_side &&
        a.exam_screen == b.exam_screen &&
        a.exam_hour == b.exam_hour &&
        a.exam_minutes == b.exam_minutes
    );
}


/*
*   Decides what to draw on the display and what not to.
*   The function ensures that whatever is already drawn
*   on the display does not get redrawn repeatedly.
*/
static void filter_request_and_draw(const DISPLAY_STATE_t& requested)
{
    bool    cluster_already_visible;

    watchdog_reset();
    if (display_cache_valid && same_display_state(display_cache, requested))
    {
        DEBUG_PRINTF("\n[THE DISPLAY] Nothing new to draw. Drawing aborted\n\n");
        return;
    }
    if (requested.view == DISPLAY_VIEW_BLANK)
    {
        display.clearScreen();
        display.writeScreenBuffer();
        display_cache = requested;
        display_cache_valid = true;
        return;
    }
    if (requested.view == DISPLAY_VIEW_EXAM)
    {
        draw_exam_screen(requested.exam_screen);
        display_cache = requested;
        display_cache_valid = true;
        return;
    }
    if (requested.view == DISPLAY_VIEW_CLUSTER)
    {
        cluster_already_visible = display_cache_valid && (display_cache.view == DISPLAY_VIEW_CLUSTER);
        if (!cluster_already_visible)
            draw_bitmap_full_update(cluster_number_img, 630, 480);
        if (!cluster_already_visible ||
            display_cache.cluster_side != requested.cluster_side ||
            display_cache.exam_hour != requested.exam_hour ||
            display_cache.exam_minutes != requested.exam_minutes)
        {
            draw_cluster_side(requested);
        }
        display_cache = requested;
        display_cache_valid = true;
        DEBUG_PRINTF("[THE DISPLAY] The drawing process is complete\n");
    }
}


static void display_draw_cluster_default(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_CLUSTER, CLUSTER_SIDE_DEFAULT, EXAM_SCREEN_NONE, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Cluster number with default cluster icons\n");
    filter_request_and_draw(state);
}

static void display_draw_cluster_intra_error(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_CLUSTER, CLUSTER_SIDE_INTRA_ERROR, EXAM_SCREEN_NONE, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Cluster number with Intra error warning\n");
    filter_request_and_draw(state);
}

static void display_draw_cluster_secret_expired(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_CLUSTER, CLUSTER_SIDE_SECRET_EXPIRED, EXAM_SCREEN_NONE, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Cluster number with Secret expiration warning\n");
    filter_request_and_draw(state);
}

static void display_draw_cluster_low_battery(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_CLUSTER, CLUSTER_SIDE_LOW_BATTERY, EXAM_SCREEN_NONE, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Cluster number with low battery warning\n");
    filter_request_and_draw(state);
}

static void display_draw_cluster_ota_waiting(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_CLUSTER, CLUSTER_SIDE_OTA_WAITING, EXAM_SCREEN_NONE, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Cluster number with OTA waiting notification\n");
    filter_request_and_draw(state);
}

static void display_draw_cluster_ota_success(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_CLUSTER, CLUSTER_SIDE_OTA_SUCCESS, EXAM_SCREEN_NONE, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Cluster number with OTA success notification\n");
    filter_request_and_draw(state);
}

static void display_draw_cluster_ota_fail(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_CLUSTER, CLUSTER_SIDE_OTA_FAIL, EXAM_SCREEN_NONE, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Cluster number with failed OTA update notification\n");
    filter_request_and_draw(state);
}

static void display_draw_cluster_ota_canceled(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_CLUSTER, CLUSTER_SIDE_OTA_CANCELED, EXAM_SCREEN_NONE, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Cluster number with canceled OTA update notification\n");
    filter_request_and_draw(state);
}

static void display_draw_cluster_telegram_error(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_CLUSTER, CLUSTER_SIDE_TELEGRAM_ERROR, EXAM_SCREEN_NONE, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Cluster number with Telegram error warning\n");
    filter_request_and_draw(state);
}

static void display_draw_cluster_exam_day(uint16_t hour, uint16_t minutes)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_CLUSTER, CLUSTER_SIDE_EXAM_DAY, EXAM_SCREEN_NONE, hour, minutes};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Cluster number with exam time notification\n");
    filter_request_and_draw(state);
}

static void display_draw_exam_pre_50(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_EXAM, CLUSTER_SIDE_NONE, EXAM_SCREEN_PRE_50, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Full-screen pre-exam warning with 50 mins left\n");
    filter_request_and_draw(state);
}

static void display_draw_exam_pre_25(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_EXAM, CLUSTER_SIDE_NONE, EXAM_SCREEN_PRE_25, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Full-screen pre-exam warning with 25 mins left\n");
    filter_request_and_draw(state);
}

static void display_draw_exam_pre_5(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_EXAM, CLUSTER_SIDE_NONE, EXAM_SCREEN_PRE_5, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Full-screen pre-exam warning with 5 mins left\n");
    filter_request_and_draw(state);
}

static void display_draw_exam_active(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_EXAM, CLUSTER_SIDE_NONE, EXAM_SCREEN_ACTIVE, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to draw: Full-screen Exam sign\n");
    filter_request_and_draw(state);
}

static void display_clear(void)
{
    DISPLAY_STATE_t state = {DISPLAY_VIEW_BLANK, CLUSTER_SIDE_NONE, EXAM_SCREEN_NONE, 0, 0};
    DEBUG_PRINTF("[THE DISPLAY] Requested to clear the display\n");
    filter_request_and_draw(state);
}


/*
*   Universal API function for drawing anything
*   on the eInk display.
*/
void    draw_on_display(IMAGE_t image)
{
    if (image == EXAM_ACTIVE)
        display_draw_exam_active();
    else if (image == PREEXAM_5)
        display_draw_exam_pre_5();
    else if (image == PREEXAM_25)
        display_draw_exam_pre_25();
    else if (image == PREEXAM_50)
        display_draw_exam_pre_50();
    else if (image == CLUSTER_DEFAULT)
        display_draw_cluster_default();
    else if (image == INTRA_ERROR)
        display_draw_cluster_intra_error();
    else if (image == SECRET_EXPIRED)
        display_draw_cluster_secret_expired();
    else if (image == EXAM_DAY)
        display_draw_cluster_exam_day(rtc_g.exam_start_hour, rtc_g.exam_start_minutes);
    else if (image == LOW_BATTERY)
        display_draw_cluster_low_battery();
    else if (image == OTA_WAITING)
        display_draw_cluster_ota_waiting();
    else if (image == OTA_SUCCESS)
        display_draw_cluster_ota_success();
    else if (image == OTA_FAIL)
        display_draw_cluster_ota_fail();
    else if (image == OTA_CANCELED)
        display_draw_cluster_ota_canceled();
    else if (image == TELEGRAM_ERROR)
        display_draw_cluster_telegram_error();
    else if (image == BLANK)
        display_clear();
    else
    {
        DEBUG_PRINTF("[THE DISPLAY] Unsupported image requested: %d\n", image);
        return;
    }
    DEBUG_PRINTF("[THE DISPLAY] The drawing process is complete\n");
}


void  display_init(void)
{
    watchdog_reset();
    SPI.end();
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_SS_PIN);
    display.init(115200);
}
 