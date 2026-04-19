
#ifndef GLOBALS_H_
# define GLOBALS_H_

# include "constants.h"
# include <Wire.h>                                  // for SPI reconfiguration in the display init function
# include <GxEPD2_BW.h>                             // we still need the black/white library version
# include <GxEPD2_3C.h>                             // 3-coloured version of the GxEPD2 library
# include <Fonts/FreeSansBold24pt7b.h>              // the fonts come from the Adafruit GFX library which gets called by the GxEPD2 library

extern GxEPD2_3C<GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT> display;           // GDEY075Z08 800x480 3-colour display

#endif
 
