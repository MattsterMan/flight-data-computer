#include <Arduino.h>
#include <SPI.h>
#include <string>
#include "Adafruit_MPL3115A2.h"
#include "RP2040_SD.h"
#include "RFTransmitter.h"
#include <EEPROM.h>

#define PIN_SD_MOSI       PIN_SPI0_MOSI
#define PIN_SD_MISO       PIN_SPI0_MISO
#define PIN_SD_SCK        PIN_SPI0_SCK
#define PIN_SD_SS         PIN_SPI0_SS

#define NODE_ID          1
#define OUTPUT_PIN       3u

Adafruit_MPL3115A2 baro;
int count;
int addr = 0;
byte value;
float init_alt;

RFTransmitter transmitter(OUTPUT_PIN, NODE_ID);

void setup() {
    //set onboard LED to be an output
    pinMode(LED_BUILTIN, OUTPUT);
    count = 0;

    //open serial port and wait for connection
    Serial.begin(9600);
    while (!Serial);
    EEPROM.begin(256);
    Serial.println("Checking for Barometer Sensor...");

    if (!baro.begin()) {
        Serial.println("Could not find sensor. Check wiring.");
    }
    else{
        Serial.println("Sensor found.");
    }

    // use to set sea level pressure for current location
    // this is needed for accurate altitude measurement
    // STD SLP = 1013.26 hPa
    baro.setSeaPressure(1024.2);

    //SD-CARD-CODE------------------------------------------------------------------------------------------------------

    /*Serial.print("Initializing SD card...");

    // see if the card is present and can be initialized:
    if (!SD.begin(PIN_SD_SS)) {
        Serial.println("Card failed, or not present");
        // don't do anything more:
        return;
    }
    Serial.println("Card initialized.");*/

    //------------------------------------------------------------------------------------------------------------------
}

void loop() {
    //set onboard LED to on
    digitalWrite(LED_BUILTIN, HIGH);

    //initial barometric reading for ground level
    if (count == 0) {
        init_alt = baro.getAltitude();
    }

    //set a variable equal to the output of the altitude reading on the MPL3115
    float altitude = baro.getAltitude();
    // write the value to the appropriate byte of the EEPROM.
    // these values will remain there when the board is
    // turned off.
    EEPROM.write(addr, altitude);
    // read a byte from the current address of the EEPROM
    value = EEPROM.read(addr);

    Serial.print(addr);
    Serial.print("\t");
    Serial.print(value, DEC);
    Serial.println();

    // advance to the next address.  there are 512 bytes in
    // the EEPROM, so go back to 0 when we hit 512.
    // save all changes to the flash.
    addr = addr + 1;
    if (addr == 16) {
        addr = 0;
        if (EEPROM.commit()) {
            Serial.println("EEPROM successfully committed");
        } else {
            Serial.println("ERROR! EEPROM commit failed");
        }
        return;
    }

    //converts altitude to string
    char* msg = std::to_string(altitude).data();
    transmitter.send((byte *)msg, strlen(reinterpret_cast<const char *>(msg)) + 1);
    Serial.println(msg);

    //set oboard LED to off
    digitalWrite(LED_BUILTIN, LOW);

    //delay(500);
    transmitter.resend((byte *)msg, strlen(reinterpret_cast<const char *>(msg)) + 1);

    count++;
}