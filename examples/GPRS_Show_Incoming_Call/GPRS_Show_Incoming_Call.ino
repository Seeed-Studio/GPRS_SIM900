/*
    SIM900 Serial Debug

    This sketch is used to communicate with SIM900 with AT commands.

    create on 2015/05/14, version: 1.0
    by lawliet.zou(lawliet.zou@gmail.com)
*/
#include "GPRS_Shield_Arduino.h"
#include <Wire.h>

const int PIN_TX = 7;
const int PIN_RX = 8;
const int BAUDRATE = 9600;

GPRS gprs(PIN_TX, PIN_RX, BAUDRATE); //RX,TX,PWR,BaudRate
char in_number[16] = {0};

void setup() {
    gprs.checkPowerUp();
    Serial.begin(9600);
}

void loop() {
    //  gprs.AT_Bypass();

    /**
        Result code:
        0: ready
        2: unknown
        3: ringing
        4: call in progress
    */
    bool is_active;
    char in_number[16] = {0};

    is_active = gprs.isCallActive(in_number);

    Serial.print("Active code : ");
    Serial.println(is_active);
    if (is_active) {
        Serial.print("Phone call in: ");
        Serial.println(in_number);
    } else {
        Serial.println("No phone call in.");
    }

    delay(1000);
}


