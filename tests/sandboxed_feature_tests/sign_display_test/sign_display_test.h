
#ifndef _42_DISPLAY_TEST_H
# define _42_DISPLAY_TEST_H

# include <Arduino.h>                     // String variables manipulations
# include <stdint.h>                      // provides fixed-width integer types
# include "bitmap_library.h"
# include "globals.h"                     // contains global variables, calls display libraries

void IRAM_ATTR        ft_display_init(void);
void                  draw_cluster_number_full_update(const unsigned char* image, uint16_t width, uint16_t height);
void                  draw_exam_note_partial_update(const unsigned char* image, uint16_t width, uint16_t height);
void                  draw_exam_start_time(void);
void                  draw_colour_bitmap_1(const unsigned char* black_image, const unsigned char* red_image);
void                  draw_colour_bitmap_2(const unsigned char* black_image, const unsigned char* red_image);
void                  draw_colour_bitmap_3(const unsigned char* black_image, const unsigned char* red_image);
void                  draw_exam_minutes_left(void);

void                  ft_draw_colour_bitmap(const unsigned char* black_image, const unsigned char* red_image);
void                  ft_display_timer_exact_end_time(void);
void                  ft_display_timer(int minutes);
static void           ft_draw_text(String output, uint16_t x, uint16_t y);
static void           ft_draw_exam_start_time(void);
static void           ft_draw_bitmap_partial_update(const unsigned char* image, uint16_t width, uint16_t height);
static void           ft_draw_bitmap_full_update(const unsigned char* image, uint16_t width, uint16_t height);
void IRAM_ATTR        ft_display_cluster_number(uint8_t mode);
bool IRAM_ATTR        ft_clear_display(bool errase_display);

#endif
 
