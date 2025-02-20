/**
 * @file    ws2in9b_v3.h
 * @author  Zachary Taylor (@ztaylor54)
 * @brief   Driver for Waveshare 2.9in 3-color e-paper display, type 'B' V3
 * @version 0.1
 * @date    2022-12-21
 *
 * @copyright GNU GPLv3
 *
 */

#pragma once
// GxEPD2 settings
#define ENABLE_GxEPD2_GFX 0
#define USE_HSPI_FOR_EPD

// GxIO_Class io(SPI, /*CS=*/SS, /*DC=*/17, /*RST=*/16); // arbitrary selection of 8, 9 selected for default of GxEPD_Class
// GxEPD_Class display(io, /*RST=*/16, /*BUSY=*/4);      // default selection of (9), 7

// display module pins
#define EPD_BUSY 40 /**< Busy status output, low active */
#define EPD_RST 16  /**< External reset, low active */
#define EPD_DC 17   /**< Data / Command selection (high for data, low for command) */
#define EPD_CS 37   /**< SPI chip selection, low active */
#define EPD_CLK 36  /**< SPI SCK pin */
#define EPD_DIN 35  /**< SPI MOSI pin */

// spi bus pins
#define SPI_SS EPD_CS
#define SPI_SCK EPD_CLK
#define SPI_MOSI EPD_DIN
#define SPI_MISO 12 /**< Unused, but you need to set it to something */

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>

GxEPD2_3C<GxEPD2_290c, GxEPD2_290c::HEIGHT> display(GxEPD2_290c(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)); /**< display driver: GDEW029Z10 128x296, UC8151 (IL0373) */

SPIClass hspi(HSPI);

void ws2in9b_v3_init()
{
    log_i("ws2in9b_v3 init");

    hspi.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_SS);
    display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));

    display.init(115200);
}