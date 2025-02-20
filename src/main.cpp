#define ENABLE_GxEPD2_GFX 0

#include <Arduino.h>
// #include <Wire.h>
// #include <esp_deep_sleep.h>
#include <stdlib.h>
#include <time.h>
#include "data.h" // Include the data file

#include <GxEPD2_BW.h> // including both doesn't use more code or ram
#include <GxEPD2_3C.h> // including both doesn't use more code or ram
#include <GxEPD2_4C.h> // including both doesn't use more code or ram
#include <GxEPD2_7C.h> // same for all three

#include <U8g2_for_Adafruit_GFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>

#define SCREEN_CS 5
#define SCREEN_DC 17
#define SCREEN_RST 16
#define SCREEN_BUSY 4

#define BUTTON_PIN 0 // GPIO0, adjust as needed

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

void displayArrayElement(int index);
void goToDeepSleep();
void handleWakeup();
void wrapText(U8G2_FOR_ADAFRUIT_GFX &u8g2Fonts, const String &text, int16_t x, int16_t y, int16_t maxWidth, int16_t lineHeight);

// GxEPD2_3C<GxEPD2_213C, GxEPD2_213C::HEIGHT> display(GxEPD2_213C(SCREEN_CS, SCREEN_DC, SCREEN_RST, SCREEN_BUSY));
GxEPD2_3C<GxEPD2_290c, GxEPD2_290c::HEIGHT> display(GxEPD2_290c(/*CS=5*/ 5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4)); // my suggested wiring and proto board

RTC_DATA_ATTR int currentIndex = 0;

void setup()
{
    // Initialize the display
    display.init();
    u8g2Fonts.begin(display);
    display.setRotation(1);

    // Initialize the button pin
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // 0 = Low, adjust as needed

    // Seed the random number generator with the current time
    srand(time(NULL));

    // Display the current array element
    displayArrayElement(currentIndex);

    // Go to deep sleep
    goToDeepSleep();
}

void loop()
{
    // Empty. We use deep sleep to wake up and handle things in the setup.
}

void wrapText(U8G2_FOR_ADAFRUIT_GFX &u8g2Fonts, const String &text, int16_t x, int16_t y, int16_t maxWidth, int16_t lineHeight)
{
    String line = "";
    String word = "";
    for (int i = 0; i < text.length(); i++)
    {
        if (text[i] == ' ' || text[i] == '\n')
        {
            if (u8g2Fonts.getUTF8Width((line + word).c_str()) > maxWidth)
            {
                u8g2Fonts.setCursor(x, y);
                u8g2Fonts.println(line);
                y += lineHeight;
                line = word + " ";
            }
            else
            {
                line += word + " ";
            }
            word = "";
            if (text[i] == '\n')
            {
                u8g2Fonts.setCursor(x, y);
                u8g2Fonts.println(line);
                y += lineHeight;
                line = "";
            }
        }
        else
        {
            word += text[i];
        }
    }
    if (u8g2Fonts.getUTF8Width((line + word).c_str()) > maxWidth)
    {
        u8g2Fonts.setCursor(x, y);
        u8g2Fonts.println(line);
        y += lineHeight;
        line = word;
    }
    else
    {
        line += word;
    }
    if (line.length() > 0)
    {
        u8g2Fonts.setCursor(x, y);
        u8g2Fonts.println(line);
    }
}

void displayArrayElement(int index)
{
    uint16_t bg = GxEPD_WHITE;
    uint16_t fg = GxEPD_BLACK;
    u8g2Fonts.setFontMode(1);                            // use u8g2 transparent mode (this is default)
    u8g2Fonts.setFontDirection(0);                       // left to right (this is default)
    u8g2Fonts.setForegroundColor(fg);                    // apply Adafruit GFX color
    u8g2Fonts.setBackgroundColor(bg);                    // apply Adafruit GFX color
    u8g2Fonts.setFont(u8g2_font_8x13O_mf);               // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
    int16_t tw = u8g2Fonts.getUTF8Width(data[index][0]); // text box width
    int16_t ta = u8g2Fonts.getFontAscent();              // positive
    int16_t td = u8g2Fonts.getFontDescent();             // negative; in mathematicians view
    int16_t th = ta - td;                                // text box height
    uint16_t x = (display.width() - tw) / 2;
    uint16_t y = (display.height() - th) / 2 + ta;
    display.fillScreen(bg);
    u8g2Fonts.setFont(u8g2_font_8x13_tf); // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
    wrapText(u8g2Fonts, data[index][1], 5, 10, display.width() - 10, th);

    u8g2Fonts.setFont(u8g2_font_8x13O_mf); // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
    wrapText(u8g2Fonts, data[index][2], 5, 50, display.width() - 10, th);

    display.display();
}

/* void displayArrayElement(int index)
{
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    // display.setCursor(0, 10);

    int16_t x1, y1;
    uint16_t w, h;
    display.setTextSize(1);
    display.setFont(&FreeSans12pt7b);
    display.getTextBounds(data[index][0], 0, 0, &x1, &y1, &w, &h);
    int16_t x = (display.width() - w) / 2;

    display.setCursor(x, 25);
    display.println(data[index][0]);

    display.fillRect(6, 34, 282, 6, GxEPD_RED);
    display.setCursor(0, 60);
    display.setTextSize(1);
    display.setFont(&FreeSans9pt7b);
    display.println(data[index][1]);
    display.setFont(&FreeSansOblique9pt7b);
    display.print(data[index][2]);
    display.display();
} */

void goToDeepSleep()
{
    // Calculate time until next day (24 hours from now)
    uint64_t timeUntilNextDay = 24 * 60 * 60 * 1000000ULL; // 24 hours in microseconds
    esp_sleep_enable_timer_wakeup(timeUntilNextDay);
    esp_deep_sleep_start();
}

// Wake up from deep sleep
void handleWakeup()
{
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0)
    {
        // Button press wake up
        currentIndex = rand() % numArrays;
    }
    else if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
    {
        // Timer wake up (end of day)
        currentIndex = rand() % numArrays;
    }
}
