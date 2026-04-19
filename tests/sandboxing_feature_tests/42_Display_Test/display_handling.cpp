
// EXTRA NOTES AT THE BOTTOM

#include "42_Display_Test.h"

    int exam_start_hour = 17;
    int exam_start_minutes = 45;


// EXPERIMENT 1: the exam reservation note is displayed immediately with the exam time
// the time text is simply not given the command to fill the screen.
// WHAT I WANT TO ACHIEVE: at least, that the exam time is displayed with the note bitmap
// in one iteration instead of two; at a maximum, that the exam time is displayed
// quickly and does not spoil the picture around it

void draw_cluster_number_full_update(const unsigned char* image, uint16_t width, uint16_t height) 
{
    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(0, 0, image, width, height, GxEPD_BLACK);
    }
    while (display.nextPage());
}

void  draw_exam_note_partial_update(const unsigned char* image, uint16_t width, uint16_t height)
{
    display.setRotation(0);
    display.setPartialWindow(630, 0, width, height);
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(630, 0, image, width, height, GxEPD_BLACK);
    }
    while (display.nextPage());
}

void  draw_exam_start_time(void)                                                // flikers and inverts colours while running
{
    String   text;
    int16_t  text_x = 27;                                                       // always points to the bottom left corner of the first(!) line of text 
    int16_t  text_y = 776;                                                      // always points to the bottom left corner of the first(!) line of text
    int16_t  window_x = 0;                                                      // always points to the top left corner of the window box 
    int16_t  window_y = text_y - 36;                                            // always points to the top left corner of the window box
    int16_t  window_width = display.width();
    int16_t  window_height = 40;

    text = "TODAY AT " + String(exam_start_hour);
    if (exam_start_minutes < 10)
        text += ":0";
    else
        text += ":";
    text += String(exam_start_minutes);
    display.setRotation(3);                                                          // rotate the whole display so the future text faces right
    display.setFont(&FreeSansBold24pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setPartialWindow(window_x, window_y, window_width, window_height);                           // create partial update window
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(text_x, text_y);
        display.print(text);
    }
    while (display.nextPage());
}






// EXPERIMENT 2: display both black-and-red pictures using both known methods.
// WHAT I WANT TO ACHIEVE: just to see the results. Maybe one method will be
// better than the other

//                      TRUE / FALSE               TRUE / FALSE               TRUE / FALSE
// For drawImage:   Flash or SRAM memory,       Show black or not,          Show red or not
// For writeImage:  Flash or SRAM memory,     Invert colours or not,    Wait till display finishes or not

void draw_colour_bitmap_1(const unsigned char* black_image, const unsigned char* red_image)
{
    display.setFullWindow();
    display.setRotation(0);
    display.drawImage(black_image, red_image, 0, 0, 800, 480, true, true, true); // black image gets inverted
}

void draw_colour_bitmap_2(const unsigned char* black_image, const unsigned char* red_image)
{
    display.setFullWindow();
    display.setRotation(0);
    display.writeImage(0, red_image, 0, 0, 800, 480, true, false, true);
    display.drawImage(black_image, 0, 0, 0, 800, 480, true, true, true);
}

void draw_colour_bitmap_3(const unsigned char* black_image, const unsigned char* red_image)
{
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
}

void  draw_exam_minutes_left(void)                                                  // flikers and inverts colours while running
{
    String   text;
    int16_t  text_x = 215;                                                       // always points to the bottom left corner of the first(!) line of text 
    int16_t  text_y = 426;                                                      // always points to the bottom left corner of the first(!) line of text
    int16_t  window_x = text_x - 4;                                                      // always points to the top left corner of the window box 
    int16_t  window_y = text_y - 33;                                            // always points to the top left corner of the window box
    int16_t  window_width = 55;
    int16_t  window_height = 40;

    text = "50";
    display.setRotation(0);                                                          // rotate the whole display so the future text faces right
    display.setFont(&FreeSansBold24pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setPartialWindow(window_x, window_y, window_width, window_height);                           // create partial update window
    display.firstPage();
    do
    {
//        display.fillScreen(GxEPD_WHITE);
        display.setCursor(text_x, text_y);
        display.print(text);
    }
    while (display.nextPage());
}























void ft_display_timer_exact_end_time(void)
{
  return;
}

void ft_display_timer(int minutes)
{
  return;
}

//================= The following function is only an example of drawing a 3-colour bitmap ====================
//================= from the library and was not yet adapted for the program ==================================

#if defined(ESP32) && defined(_GxBitmaps3c1304x984_H_)
void drawBitmaps3c1304x984()
{
    display.setFullWindow();
    display.setRotation(0);
    //display.drawImage(Bitmap3c1304x984_black, Bitmap3c1304x984_red, 0, 0, 1304, 984, false, false, true);
    display.writeImage(0, Bitmap3c1304x984_red, 0, 0, 1304, 984, true, false, true); // red bitmap is inverted
    display.drawImage(Bitmap3c1304x984_black, 0, 0, 0, 1304, 984, true, false, true); // black bitmap is normal
}
#endif

//=============================================================================================================

// THIS FUNCTION IS CONFIGURED TO ACCURATELY DISPLAY THE EXAM TIME IN THE WARNING NOTE

static void  ft_draw_text(String output, uint16_t x, uint16_t y)                             // flikers and inverts colours while running
{
    int16_t         text_box_x;
    int16_t         text_box_y;
    uint16_t        text_width;
    uint16_t        text_height;

    display.setRotation(3);                                                                   // rotate the whole display so the future text faces right
    display.setFont(&FreeSansBold24pt7b);
    display.setTextColor(GxEPD_BLACK);
//    display.getTextBounds(output, 0, 0, &text_box_x, &text_box_y, &text_width, &text_height);       // check the dimensions of the future text
    display.setPartialWindow(224, 732, 140, 40);                                                      // create partial update window
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y);
        display.print(output);
    }
    while (display.nextPage());
}

//=============================================================================================================



void ft_draw_colour_bitmap(const unsigned char* black_image, const unsigned char* red_image)
{
    display.setFullWindow();
    display.setRotation(0);
    display.drawImage(black_image, red_image, 0, 0, 800, 480, false, false, true);
}

static void  ft_draw_exam_start_time(void)                                            // flikers and inverts colours while running
{
    String   text;
    int16_t  text_x = 27;                                                       // always points to the bottom left corner of the first(!) line of text 
    int16_t  text_y = 776;                                                      // always points to the bottom left corner of the first(!) line of text
    int16_t  window_x = 0;                                                      // always points to the top left corner of the window box 
    int16_t  window_y = text_y - 36;                                            // always points to the top left corner of the window box
    int16_t  window_width = display.width();
    int16_t  window_height = 40;

    text = "TODAY AT " + String(exam_start_hour);
    if (exam_start_minutes < 10)
        text += ":0";
    else
        text += ":";
    text += String(exam_start_minutes);
    display.setRotation(3);                                                          // rotate the whole display so the future text faces right
    display.setFont(&FreeSansBold24pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setPartialWindow(window_x, window_y, window_width, window_height);                           // create partial update window
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(text_x, text_y);
        display.print(text);
    }
    while (display.nextPage());
}

static void ft_draw_bitmap_partial_update(const unsigned char* image, uint16_t width, uint16_t height)    // with display refresh: flickers, does NOT invert colours while running 
{                                                                                                         // this function only draws the cluster slides
    display.setRotation(0);
    display.setPartialWindow(630, 0, width, height);
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(630, 0, image, width, height, GxEPD_BLACK);
    }
    while (display.nextPage());
}

static void ft_draw_bitmap_full_update(const unsigned char* image, uint16_t width, uint16_t height)     // with display refresh: flickers, does NOT invert colours while running 
{                                                                                                       // this function only draws the cluster slides
    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(0, 0, image, width, height, GxEPD_BLACK);
    }
    while (display.nextPage());
}

bool IRAM_ATTR   ft_clear_display(bool errase_display)                          // flickers
{
    if (errase_display)
    {
        display.clearScreen();
        display.writeScreenBuffer();
        errase_display = false;
    }
    return (errase_display);
}

void IRAM_ATTR  ft_display_init(void)
{
    SPI.end();
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_SS_PIN);
    display.init(115200);
}






 
/*
 *  We need to be able to create a total number of 13 different slides. For that we have 
 *  6 bitmaps and system fonts. The bitmaps are fixed images that can be placed staticaly 
 *  anywhere on the display. With the system fonts we can write any phrases and texts.
 *  Here is the list of the slides and what each of them shall consist of:
 *      - boot_up_img                         system fonts only
 *      - empty_battery_img                   system fonts only
 *      - default_cluster_number_img          cluster_number_img + default_cluster_icons
 *      - intra_error_img                     cluster_number_img + system fonts for the error note
 *      - cluster_number_with_exam_note       cluster_number_img + system fonts for the exam note
 *      - low_battery_img                     cluster_number_img + system fonts for the low battery note
 *      - ota_waiting_img                     cluster_number_img + system fonts for the OTA waiting note
 *      - ota_updating_img                    cluster_number_img + system fonts for the OTA updating note
 *      - ota_success_img                     cluster_number_img + system fonts for the OTA success note
 *      - ota_fail_img                        cluster_number_img + system fonts for the OTA fail note
 *      - ota canceled                        cluster_number_img + system fonts for the OTA cancel note
 *      - exam_warning_img                    exam_warning_img_0 + exam_warning_img_1 + system fonts for the exam time note
 *      - preexam_warning_img                 preexam_warning_img + preexam_warning_icons + system fonts for the exam time note
 *  You may see what the final slides look should be like in the Slides Final Look folder.
 *  The slides may be devided into 4 groups by the maner of their assembly:
 *      - 2 slides that consist of system fonts only,
 *      - 8 slides that consist of the cluster_number and some note in system font,
 *      - 1 slide of preexam warning that has a unique assembly steps,
 *      - 1 slide of exam warning that has a unique assembly steps.
 *  It means that we will need at least 4 different assembly functions to take care of each
 *  of the slides groups.
 */

/*

NOTES ON COORDINATES, SIZES IN PIXELS AND ROTATIONS

In the setRotation(uint8) function you may set how the window you want to output on the display will be turned compared to the display
physical orientation. All the displays have their default physical orientation, but it may vary from display to display, so finding out
the display's default physical orientation is the starting point for any work with display graphics. It is easy to do: take a look at
the display's physical appearence, note the side where a ribbon of wires sticks out of it — that's the bottom side of this
display's default physical orientation. Knowing that, we now can set the rotation:

    0 — for the default rotation, it is equal to the display default physical orientation
    1 — will rotate the window 90 degrees clockwise comparing to the display default physical orientation
    2 — will rotate the window 180 degrees (or simply flips it up-side-down) comparing to the display default physical orientation
    3 — will rotate the window 270 degrees clockwise comparing to the display default physical orientation

Keep in mind that however you rotate the window, its system of coordinates always rotates with it. So, the 0,0 coordinates are always in
the TOP LEFT corner of your window. 

When dealing with the Partial Update windows, it is important to remember about one unobvious limitation of the e-paper controller:
partial update window size and position is on byte boundary with physical x direction. Meaning that the controller may set
the horisontal window boundaries only on every 8th pixel comparing to its default physical orientation. Let's summon it up with a simple
rule that is easy to follow:
    x and width should be multiple of 8, for rotation 0 or 2,
    y and hight should be multiple of 8, for rotation 1 or 3.

The function setPartialWindow(x, y, width, height) allows us to create on our display a window of any size and, instead of the whole
display, update only this window. It is very useful feature since it is much faster than the conventional full display update with
the setFullWindow() function. Moreover this type of an update does not flicker. So, if you need to get rid off of the flickers
between the slides, with setPartialWindow(x, y, width, height) you may set the whole display as a Partial Update window. But be
careful as THE FIRST UPDATE AFTER TURNING ON OR RESET SHALL ALWAYS BE THE FULL DISPLAY UPDATE with the setFullWindow() function.
Otherwise the display may be irrevertably stuck on the last image for ever.

It may be useful to note the relationship between the setPartialWindow(x, y, width, height) and the setRotation(uint8) functions.
Some may think that setRotation(uint8) may turn a Partial Update window created with the setPartialWindow(x, y, width, height) function.
And it is true, but only to some extent. setRotation(uint8) does not turn Partial Update windows, but it actually turns the whole display window
orientation with Partial Update windows inside of it. That is why when we change the display orientation with the setRotation(uint8) function,
we also need to remember that the system of coordinates for x and y in the setPartialWindow(x, y, width, height) also changes. And when
the system of coordinates changes, the understanding of width and height changes with it.
E.g. if while setRotation(0) the window's width is 800 and the window's height is 480, then while setRotation(1) the window's width becomes 480 
and the window's height becomes 800.

For some unexplainable reason when it comes to the text body coordinates, their logic is very different from windows and images. The x and y 
coordinates point to the bottom left corner of the first line of text even if there are multiple lines of text. In comparison, the common
logic for the windows and bitmaps coordinates are that their x and y point to the top left corner of the image.
So, if you used display.setCursor(x, y) command to output the word HELLO onto the display, the x and the y there would point to
the very bottom left pixel of the letter H.

The coordinates logic is also tricky when it comes to the command
display.getTextBounds(output, 0, 0, &text_box_x, &text_box_y, &text_width, &text_height)
This command can view any text as the smallest box that can accomodate the given text and give you all the needed information to create
such a box on the screen. It is very useful when you want to update a piece of text or a single character on your display instead of the whole
display. The function takes a few parameters: the text in the form of a String varible (here it is "output"), x and y coordinates of the area
you want to place the text (here they are "0, 0" - the top left corner of the display), pointers to x and y coordinates (this is how you will get
x and y coordinates for the top left corner of the box for the text), pointers to the width and height (this is how you will get width and height
values of the box for the text).
It is not required to change the "0, 0" coordinates because this function does not place anything anywhere - it uses this values only for
calculations. Though, it is important to remember, that when you use the default "0, 0" coordinates, the value in the text_box_y variable
will be NEGATIVE. Why does this happen? Exactly because of the difference in the coordinates logic between texts and windows/bitmaps/boxes.
If you remember, x and y for texts point to the BOTTOM left pixel of the text; but x and y for windows/bitmaps/boxes point to their TOP left
pixel. The display.getTextBounds tries to compensate this logic difference. E.g. if you were to place the word HELLO with the 10 size font into
the top left corner (0, 0) of the display, you would need to call setCursor(0, 10); but if you were to creare a partial update window in the
the top left corner (0, 0) of the display to update that text, you would need to call setPartialWindow(0, 0, text_width, 10). But noone has time
to deal with different coordinates logic! I want to give the function the text and just one set of x,y coordinates and the text shall simply
aline with my partial update window automatically! That's where the getTextBounds may help. In our example our target display coordinates are
x = 0, y = 0 (the top left corner), so let's place HELLO in there with a partial display update:
     display.setTextSize(10);
     display.getTextBounds("HELLO", 0, 0, &text_box_x, &text_box_y, &text_width, &text_height);
        // now text_box_x has value 0, text_box_y == -10, text_width == HELLO-width value (I don't know it), text_height == 10
     display.setPartialWindow(x, y, text_width, text_height);
        // that creates a window in the top left corner of the display, HELLO-width pixels wide and 10 pixels high
     display.setCursor(x - text_box_x, y - text_box_y);
        // this is how we compensate for the text coordinates logic: for x (0 - 0 = 0) and for y (0 - (-10) = 10) coordinates
     display.print("HELLO");
 */

 
