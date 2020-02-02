/*
    GPRS Call Up

    This sketch is used to test seeeduino GPRS_Shield's callUp function.
    to make it work, you should insert SIM card to Seeeduino GPRS
    and replace the phoneNumber,enjoy it!

    create on 2015/05/14, version: 1.0
    by lawliet.zou(lawliet.zou@gmail.com)
*/
#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#define PIN_TX    7
#define PIN_RX    8
#define BAUDRATE  9600
#define PHONE_NUMBER  "567***677"

GPRS gprs(PIN_TX, PIN_RX, BAUDRATE); //RX,TX,PWR,BaudRate

void setup() {
    gprs.checkPowerUp();
    Serial.begin(9600);
    while (!gprs.init()) {
        delay(1000);
        Serial.println("Initialization failed!");
    }

    while (!gprs.isNetworkRegistered()) {
        delay(1000);
        Serial.println("Network has not registered yet!");
    }

    Serial.println("gprs initialize done");
    Serial.println("start to call ...");
    gprs.callUp(PHONE_NUMBER);
}

void loop() {
    //nothing to do
}
