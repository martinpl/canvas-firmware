#ifndef _DISPLAY_EPD_W21_SPI_
#define _DISPLAY_EPD_W21_SPI_
#include "Arduino.h"

// IO settings
// SCK--GPIO23(SCLK)
// SDIN---GPIO18(MOSI)
#define isEPD_W21_BUSY digitalRead(PIN_BUSY)
#define EPD_W21_RST_0 digitalWrite(PIN_RES, LOW)
#define EPD_W21_RST_1 digitalWrite(PIN_RES, HIGH)
#define EPD_W21_DC_0 digitalWrite(PIN_DC, LOW)
#define EPD_W21_DC_1 digitalWrite(PIN_DC, HIGH)
#define EPD_W21_CS_0 digitalWrite(PIN_CS, LOW)
#define EPD_W21_CS_1 digitalWrite(PIN_CS, HIGH)

void SPI_Write(unsigned char value);
void EPD_W21_WriteDATA(unsigned char datas);
void EPD_W21_WriteCMD(unsigned char command);

#endif