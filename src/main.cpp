#define ENABLE_GxEPD2_GFX 0

#include <Arduino.h>
// #include <Wire.h>
// #include <esp_deep_sleep.h>
#include <stdlib.h>
#include <time.h>
#include "data.h" // Include the data file

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_4C.h>
#include <GxEPD2_7C.h>

#include <U8g2_for_Adafruit_GFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>

#define SCREEN_CS 5
#define SCREEN_DC 17
#define SCREEN_RST 16
#define SCREEN_BUSY 4

#define BUTTON_PIN 33 // GPIO0, adjust as needed

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

void displayArrayElement(int index);
void goToDeepSleep();
void handleWakeup();
int16_t wrapText(U8G2_FOR_ADAFRUIT_GFX &u8g2Fonts, const String &text, int16_t x, int16_t y, int16_t maxWidth, int16_t lineHeight);
void renderCenteredText(U8G2_FOR_ADAFRUIT_GFX &u8g2Fonts, const String &text, int16_t y);
int getRandomIndex();

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
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0); // 0 = Low, adjust as needed

    // Seed the random number generator with a random value from esp_random()
    srand(esp_random());

    currentIndex = getRandomIndex();

    // Handle wakeup
    handleWakeup();

    uint16_t bg = GxEPD_WHITE;
    uint16_t fg = GxEPD_BLACK;
    u8g2Fonts.setForegroundColor(fg); // apply Adafruit GFX color
    u8g2Fonts.setBackgroundColor(bg); // apply Adafruit GFX color

    display.fillScreen(bg);

    // Display the current array element
    displayArrayElement(currentIndex);

    // Go to deep sleep
    goToDeepSleep();
}

void loop()
{
    // Empty. We use deep sleep to wake up and handle things in the setup.
}

int16_t wrapText(U8G2_FOR_ADAFRUIT_GFX &u8g2Fonts, const String &text, int16_t x, int16_t y, int16_t maxWidth, int16_t lineHeight)
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
        y += lineHeight;
    }
    return y;
}

void renderCenteredText(U8G2_FOR_ADAFRUIT_GFX &u8g2Fonts, const String &text, int16_t y)
{
    int16_t tw = u8g2Fonts.getUTF8Width(text.c_str());
    int16_t ta = u8g2Fonts.getFontAscent();
    int16_t td = u8g2Fonts.getFontDescent();
    int16_t th = ta - td;
    uint16_t x = (display.width() - tw) / 2;
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.println(text);
}

void displayArrayElement(int index)
{
    uint8_t lineHeight = 12;

    u8g2Fonts.setFont(u8g2_font_profont22_tf); // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
    renderCenteredText(u8g2Fonts, data[index][0], 16);

    u8g2Fonts.setFont(u8g2_font_8x13_tf); // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
    int16_t y = wrapText(u8g2Fonts, data[index][1], 5, 32, display.width() - 6, lineHeight);

    u8g2Fonts.setFont(u8g2_font_8x13O_mf); // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
    wrapText(u8g2Fonts, data[index][2], 5, y + (lineHeight / 2), display.width() - 6, lineHeight);

    display.display();
}

void goToDeepSleep()
{
    // Calculate time until next day (24 hours from now)
    uint64_t timeUntilNextDay = 24 * 60 * 60 * 1000000ULL; // 24 hours in microseconds
    esp_sleep_enable_timer_wakeup(timeUntilNextDay);
    esp_deep_sleep_start();
}

int getRandomIndex()
{
    return rand() % numArrays;
}

// Wake up from deep sleep
void handleWakeup()
{
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0)
    {
        // Button press wake up
        currentIndex = getRandomIndex();
    }
    else if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
    {
        // Timer wake up (end of day)
        currentIndex = getRandomIndex();
    }
}
