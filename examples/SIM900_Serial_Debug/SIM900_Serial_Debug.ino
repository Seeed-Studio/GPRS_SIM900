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

void setup() {
    gprs.checkPowerUp();
    Serial.begin(9600);
}

void loop() {
    gprs.AT_Bypass();
}



