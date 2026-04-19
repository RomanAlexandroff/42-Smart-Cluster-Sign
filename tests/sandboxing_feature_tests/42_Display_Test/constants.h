
#ifndef CONSTANTS_H
# define CONSTANTS_H

# define BUSY_PIN                19
# define RST_PIN                 18                                    // also known as RES pin
# define DC_PIN                  0
# define SPI_SS_PIN              10                                    // also known as CS pin
# define SPI_MOSI_PIN            7                                     // also known as SDA pin, as SPI_D pin or as DIN pin
# define SPI_SCK_PIN             6                                     // also known as SCL pin
# define SPI_MISO_PIN            -1                                    // NOT USED IN THIS PROJECT
# define GxEPD2_DISPLAY_CLASS    GxEPD2_3C
# define GxEPD2_DRIVER_CLASS     GxEPD2_750c_Z08
# define GxEPD2_3C_IS_GxEPD2_3C  true
# define IS_GxEPD2_3C(x)         IS_GxEPD(GxEPD2_3C_IS_, x)
# define MAX_DISPLAY_BUFFER_SIZE 96124ul                               // recomended buffer size for 3-colour 800x480 e-paper display — 96124 Bytes
# define MAX_HEIGHT(EPD)         (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))

#endif
 
