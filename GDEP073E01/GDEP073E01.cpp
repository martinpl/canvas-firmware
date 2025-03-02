#include "Display_EPD_W21_spi.cpp"
#include "Display_EPD_W21.cpp"

void displayInChunks(int firstOrLast, const int bytesRead, const unsigned char *picData)
{
    if (firstOrLast == 1)
    {
        EPD_init_fast();
        EPD_W21_WriteCMD(0x10);
    }

    unsigned int i;
    unsigned char dataH, dataL;
    for (i = 0; i < bytesRead; i += 2)
    {
        dataH = picData[i] << 4;
        dataL = picData[i + 1];
        EPD_W21_WriteDATA(dataH | dataL);
    }

    if (firstOrLast == 9)
    {
        // Refresh
        EPD_W21_WriteCMD(0x12); // DISPLAY REFRESH
        EPD_W21_WriteDATA(0x00);
        delay(1);        //!!!The delay here is necessary, 200uS at least!!!
        lcd_chkstatus(); // waiting for the electronic paper IC to release the idle signal
        EPD_sleep();
    }
}
