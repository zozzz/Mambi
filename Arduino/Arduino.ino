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
#define LED_NUM     68
#define LED_TYPE    NEOPIXEL
#define LED_PIN     3
#define COLOR_TYPE  CRGB
#define COLOR_SIZE  3
#define BRIGHTNESS  200
#define TIMEOUT     5000 // ms
#define DEBUG       1


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// PROGRAM SECTION
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if DEBUG
#  define Print(x) Serial.println(x); Serial.flush()
#else
#  define Print(x)
#endif

#define MAX_UPDATE_PAYLOAD_SIZE (LED_NUM * COLOR_SIZE)

COLOR_TYPE leds[LED_NUM];

// HEADER: MAMBI[LENGTH][CMD_*]
byte HEADER[] = { 0x4D, 0x41, 0x4D, 0x42, 0x49 }; // MAMBI
COLOR_TYPE TRANS_TO[LED_NUM];
COLOR_TYPE TRANS_FROM[LED_NUM];
#define HEADER_LENGTH    5
#define HEADER_READ_FAIL 0x00

// FORMAT: MAMBI[LENGTH][CMD_TURN_OFF]
#define CMD_TURN_OFF     0x01

// FORMAT: MAMBI[LENGTH][CMD_UPDATE][colors...]
#define CMD_UPDATE       0x02

// FORMAT: MAMBI[LENGTH][CMD_BRIGHTNESS][brightness: uint8_t]
#define CMD_BRIGHTNESS   0x03

// FORMAT: MAMBI[LENGTH][CMD_TRANSITION][duration: uint16_t][colors...]
#define CMD_TRANSITION   0x04
#define IS_TIMEOUT()     (millis() - beginTime >= TIMEOUT)
#define min(a, b)        ((a) > (b) ? (b) : (a))

#define TRANS_NONE       0
#define TRANS_PROGRESS   1

unsigned long beginTime;
uint8_t headerReaded = 0;
uint8_t payloadSize = 0;
uint8_t payloadConsumed = 0;

uint8_t         transState = TRANS_NONE;
unsigned long   transEnd;
uint8_t         transProgress;
uint16_t        transDuration = 0;


void setup() {
    beginTime = millis();

    FastLED.addLeds<LED_TYPE, LED_PIN>(leds, LED_NUM);
    FastLED.setBrightness(BRIGHTNESS);
    Serial.begin(115200);
    Serial.setTimeout(TIMEOUT);

    // while (!Serial); // Wait for serial port to ready
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

        case CMD_TRANSITION:
            CommandTransition();
        break;
    }

    while (payloadConsumed < payloadSize)
    {
        if (Serial.available() > 0) {
            Serial.read();
            payloadConsumed++;
        } else {
            Serial.peek();
        }
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
    transState = TRANS_NONE;
    for (uint8_t i = 0; i < LED_NUM; ++i)
    {
        leds[i] = COLOR_TYPE::Black;
    }

    FastLED.show();
    beginTime = millis();
}


void CommandUpdate()
{
    transState = TRANS_NONE;
    payloadConsumed = Serial.readBytes((byte*)leds, min(MAX_UPDATE_PAYLOAD_SIZE, payloadSize));
    FastLED.show();
    beginTime = millis();
}


void CommandTransition()
{
    if (Serial.readBytes((byte*)&transDuration, sizeof(uint16_t)) != sizeof(uint16_t))
    {
        return;
    }

    payloadConsumed = Serial.readBytes((byte*)TRANS_TO, min(MAX_UPDATE_PAYLOAD_SIZE, payloadSize - sizeof(uint16_t))) + sizeof(uint16_t);
    memcpy(TRANS_FROM, leds, sizeof(COLOR_TYPE) * LED_NUM);

    transState = TRANS_PROGRESS;
    beginTime = millis();
    transEnd = beginTime + transDuration;

    uint8_t ledNum;
    while (transState == TRANS_PROGRESS && transEnd > beginTime)
    {
        transProgress = (float)(transDuration - (transEnd - beginTime)) / (float)transDuration * 255;

        for (ledNum = 0; ledNum < LED_NUM; ++ledNum)
        {
            /*
            leds[ledNum].r = lerp8by8(TRANS_FROM[ledNum].r, TRANS_TO[ledNum].r, transProgress * 255);
            leds[ledNum].g = lerp8by8(TRANS_FROM[ledNum].g, TRANS_TO[ledNum].g, transProgress * 255);
            leds[ledNum].b = lerp8by8(TRANS_FROM[ledNum].b, TRANS_TO[ledNum].b, transProgress * 255);
            */

           Gradient(TRANS_FROM + ledNum, TRANS_TO +ledNum, transProgress, leds + ledNum);
        }

        FastLED.show();
        delay(1);
        beginTime = millis();
    }
}


#define Gradient_Lerp(__from, __to, __pos) ((__from) > (__to) \
    ? (__from) - (__from - __to) * (__pos) \
    : (__from) + (__to - __from) * (__pos))

inline void Gradient(COLOR_TYPE* _from, COLOR_TYPE* _to, uint8_t _pos, COLOR_TYPE* _result)
{
    _result->r = lerp8by8(_from->r, _to->r, _pos);
    _result->g = lerp8by8(_from->g, _to->g, _pos);
    _result->b = lerp8by8(_from->b, _to->b, _pos);
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
