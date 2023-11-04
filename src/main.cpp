#include <Arduino.h>
#include <ResponsiveAnalogRead.h>
#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>
#include <EEPROM.h>
#ifdef USB_MIDI
#include <MIDI.h>
#include <MIDIUSB.h>
MIDI_CREATE_DEFAULT_INSTANCE();
#endif

#define KNOBS 8
#define PIXELS 8
#define PIXELS_PIN 23
#define BUTTONS 8
Adafruit_NeoPixel pixels (PIXELS, PIXELS_PIN,  NEO_GRB + NEO_KHZ800);

struct OnedayState {
    bool buttons[8];
    uint16_t knobs[8];
    uint16_t crc = 0x0;
} onedayState;

Bounce buttonArray[BUTTONS] = {
        Bounce(0,25),
        Bounce(1,25),
        Bounce(2,25),
        Bounce(3,25),
        Bounce(7,25),
        Bounce(8,25),
        Bounce(9,25),
        Bounce(10,25),

};
ResponsiveAnalogRead adcArray[KNOBS] = {
        ResponsiveAnalogRead (A3, true),
        ResponsiveAnalogRead (A2, true),
        ResponsiveAnalogRead (A1, true),
        ResponsiveAnalogRead (A0, true),
        ResponsiveAnalogRead (A7, true),
        ResponsiveAnalogRead (A6, true),
        ResponsiveAnalogRead (A5, true),
        ResponsiveAnalogRead (A4, true),
};

void midiHandleClock(){}
void midiHandleStart(){}
void midiHandleContinue(){}
void midiHandleStop(){}

void setup()
{
#ifdef USB_MIDI
    MIDI.begin();
    usbMIDI.setHandleClock(midiHandleClock);
    usbMIDI.setHandleStart(midiHandleStart);
    usbMIDI.setHandleContinue(midiHandleContinue);
    usbMIDI.setHandleStop(midiHandleStop);
#endif

    EEPROM.get(0x0, onedayState);

    for (auto &anal : adcArray) {
        anal.enableEdgeSnap();
        anal.setSnapMultiplier(8.0);
    }

    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(A4, INPUT);
    pinMode(A5, INPUT);
    pinMode(A6, INPUT);
    pinMode(A7, INPUT);

    pinMode(3, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);
    pinMode(1, INPUT_PULLUP);
    pinMode(0, INPUT_PULLUP);
    pinMode(7, INPUT_PULLUP);
    pinMode(8, INPUT_PULLUP);
    pinMode(9, INPUT_PULLUP);
    pinMode(10, INPUT_PULLUP);

    Serial.begin(11520);
    Serial.println("start");
    pixels.begin();
}
void loop()
{
    for (uint8_t i = 0; i < KNOBS; i++){
        adcArray[i].update();

        if (adcArray[i].hasChanged()) {
            auto rawValue = adcArray[i].getValue();
            auto normalizedValue = map(rawValue, 0, 1023, 127, 0);
#if USB_SERIAL
            Serial.print("ADC ");
            Serial.print(i);
            Serial.print(" value: ");
            Serial.print(rawValue);
            Serial.print(" normalized: ");
            Serial.println(normalizedValue);
#else
            usbMIDI.sendControlChange(101 + i, normalizedValue, 1);
#endif
            pixels.setPixelColor(i, pixels.Color(normalizedValue,normalizedValue,normalizedValue));
            pixels.show();
        }
    }
    for (uint8_t i = 0; i < BUTTONS; i++){
        buttonArray[i].update();
        if (buttonArray[i].fell()) {
#if USB_SERIAL
            Serial.print("Button: ");
            Serial.print(i);
            Serial.println(" fell");
#else
            usbMIDI.sendControlChange(119 + i, 1, 1);
#endif
        }
        if (buttonArray[i].rose()) {
#if USB_SERIAL
            Serial.print("Button: ");
            Serial.print(i);
            Serial.println(" rose");
#else
            usbMIDI.sendControlChange(119 + i, 0, 1);
#endif
        }
    }
}