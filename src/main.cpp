#include <Arduino.h>
#include <SPI.h>
#include <string>
#include "Adafruit_MPL3115A2.h"
//#include "RP2040_SD.h"
#include "RFTransmitter.h"

/*#define PIN_SD_MOSI       PIN_SPI0_MOSI
#define PIN_SD_MISO       PIN_SPI0_MISO
#define PIN_SD_SCK        PIN_SPI0_SCK
#define PIN_SD_SS         PIN_SPI0_SS

#define _RP2040_SD_LOGLEVEL_       0*/

#define NODE_ID          1
#define OUTPUT_PIN       3u

Adafruit_MPL3115A2 baro;
int count;
float init_alt;

RFTransmitter transmitter(OUTPUT_PIN, NODE_ID);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    count = 0;

    Serial.begin(9600);
    while (!Serial);
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

    Serial.print("Initializing SD card...");

    // see if the card is present and can be initialized:
    if (!SD.begin(PIN_SD_SS)) {
        Serial.println("Card failed, or not present");
        // don't do anything more:
        return;
    }
    Serial.println("Card initialized.");
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    // make a string for assembling the data to log:
    /*String dataString = "";

    //take the altitude value from the MPL3115A2
    //and add it to the string "dataString"
    float altitude = baro.getAltitude();
    dataString += String(altitude);

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
        dataFile.println(dataString);
        dataFile.close();
        // print to the serial port too:
        Serial.println(dataString);
    }
        // if the file isn't open, pop up an error:
    else {
        Serial.println("error opening datalog.txt");
    }
    Serial.print(altitude);*/

    if (count == 0) {
        init_alt = baro.getAltitude();
    }

    float altitude = baro.getAltitude() /*- init_alt*/;

    char* msg = std::to_string(altitude).data();
    transmitter.send((byte *)msg, strlen(reinterpret_cast<const char *>(msg)) + 1);
    Serial.println(msg);

    digitalWrite(LED_BUILTIN, LOW);

    delay(500);
    transmitter.resend((byte *)msg, strlen(reinterpret_cast<const char *>(msg)) + 1);

    count++;
}