/*
    GPRS Connect TCP

    This sketch is used to test seeeduino GPRS_Shield's send SMS func.
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
#define PHONE_NUMBER "183******27"
#define MESSAGE  "hello,world"

GPRS gprs(PIN_TX, PIN_RX, BAUDRATE); //RX,TX,BaudRate

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

    Serial.println("gprs initialize done!");
    Serial.println("start to send message ...");

    if (gprs.sendSMS(PHONE_NUMBER, MESSAGE)) { //define phone number and text
        Serial.print("Send SMS Succeed!\r\n");
    } else {
        Serial.print("Send SMS failed!\r\n");
    }
}

void loop() {
    //nothing to do
}