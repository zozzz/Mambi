#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <FastLED.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CONFIG SECTION
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define LED_NUM     10
#define LED_TYPE    NEOPIXEL
#define LED_PIN     3
#define COLOR_TYPE  CRGB
#define COLOR_SIZE  3
#define BRIGHTNESS  10
#define TIMEOUT     5000 // ms


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// PROGRAM SECTION
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

COLOR_TYPE leds[LED_NUM];


// HEADER: MAMBI[LENGTH][CMD_*]
byte HEADER[] = { 0x4D, 0x41, 0x4D, 0x42, 0x49 }; // MAMBI
#define HEADER_LENGTH    5
#define HEADER_READ_FAIL 0x00
#define CMD_TURN_OFF     0x01
#define CMD_UPDATE       0x02
#define CMD_BRIGHTNESS   0x03
#define IS_TIMEOUT()     (millis() - beginTime >= TIMEOUT)

unsigned long beginTime;
uint8_t headerReaded = 0;
uint8_t payloadSize = 0;
uint8_t payloadConsumed = 0;


void setup() {
    beginTime = millis();

    FastLED.addLeds<LED_TYPE, LED_PIN>(leds, LED_NUM);
    FastLED.setBrightness(BRIGHTNESS);
    Serial.begin(115200);
}


void loop()
{
    switch (WaitForHeader())
    {
        case HEADER_READ_FAIL:
            if (IS_TIMEOUT())
            {
                CommandTurnOff();
            }
            else
            {
                return;
            }
        break;

        case CMD_TURN_OFF:
            CommandTurnOff();
        break;

        case CMD_UPDATE:
            CommandUpdate();
        break;

        case CMD_BRIGHTNESS:
            CommandBrightness();
        break;
    }

    while (payloadConsumed < payloadSize)
    {
        Serial.read();
        payloadConsumed++;
    }
}


uint8_t WaitForHeader()
{
    payloadSize = 0;

    while (headerReaded < HEADER_LENGTH)
    {
        if (Serial.available() > 0)
        {
            if (Serial.read() == HEADER[headerReaded])
            {
                headerReaded++;
            }
            else
            {
                headerReaded = 0;
                return HEADER_READ_FAIL;
            }
        }
        else
        {
            return HEADER_READ_FAIL;
        }
    }

    headerReaded = 0;
    beginTime = millis();
    while (Serial.available() >= 2 && !IS_TIMEOUT())
    {
        payloadSize = Serial.read();
        return Serial.read();
    }

    return HEADER_READ_FAIL;
}


void CommandTurnOff()
{
    for (uint8_t i = 0; i < LED_NUM; ++i)
    {
        leds[i] = COLOR_TYPE::Black;
    }

    FastLED.show();
    beginTime = millis();
}


void CommandUpdate()
{
    payloadConsumed = 0;
    uint8_t ledNum = 0;
    while (payloadConsumed < payloadSize && ledNum < LED_NUM)
    {
        ReadColor(&leds[ledNum++]);
        payloadConsumed += COLOR_SIZE;
    }

    while (ledNum < LED_NUM)
    {
        leds[ledNum++] = COLOR_TYPE::Black;
    }

    FastLED.show();
    beginTime = millis();
}


void CommandBrightness()
{
    if (payloadSize > 0)
    {
        FastLED.setBrightness(Serial.read());
        FastLED.show();
    }
    beginTime = millis();
}


void ReadColor(COLOR_TYPE* into)
{
    if (Serial.available() >= COLOR_SIZE)
    {
        into->r = Serial.read();
        into->g = Serial.read();
        into->b = Serial.read();
    }
    else
    {
        into->r = 0;
        into->g = 0;
        into->b = 0;
    }
}
