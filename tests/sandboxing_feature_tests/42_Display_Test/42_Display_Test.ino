
#include "42_Display_Test.h"

void  setup(void)
{
    Serial.begin(115200);
    delay(500);
    ft_display_init();
}

void  loop(void)
{
    draw_cluster_number_full_update(cluster_number_img, 630, 480);        // draws the cluster number
    draw_exam_note_partial_update(reserve_note_img, 170, 480);            // shows system messages and exam notes
    delay(5000);
    draw_exam_start_time();                                               // shows exact time (currently fixed to 17:45)
    
    delay(20000);
/*    
    draw_colour_bitmap_1(preexam_warning_black, preexam_warning_red);     // works, but black is inverted, the image appears mirrored horizontally
    draw_colour_bitmap_1(exam_warning_black, exam_warning_red);           // works, but red is inverted, the image appears mirrored horizontally
    delay(50000);
    
    draw_colour_bitmap_2(preexam_warning_black, preexam_warning_red);     // works, but black is inverted and there's no red, the image appears mirrored horizontally
    delay(50000);
    draw_colour_bitmap_2(exam_warning_black, exam_warning_red);           // works, but black is inverted and there's no red, the image appears mirrored horizontally
    delay(50000);
*/    
    draw_colour_bitmap_3(preexam_warning_black, preexam_warning_red);     // works great
    delay(1000);
    draw_exam_minutes_left();
    
    delay(50000);
    draw_colour_bitmap_3(exam_warning_black, exam_warning_red);           // works, but black is inverted, no other issues 
    
    delay(90000);
    
    ft_clear_display(true);
    delay(30000);
}
 
