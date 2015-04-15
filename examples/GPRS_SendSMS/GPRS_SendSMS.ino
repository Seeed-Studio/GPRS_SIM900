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
#define PHONE_NUMBER "186****2964"
#define MESSAGE  "hello,world"

GPRS gprsTest(PIN_TX,PIN_RX,BAUDRATE);//RX,TX,BaudRate

void setup() {
  Serial.begin(9600);
  while(!gprsTest.init()) {
      delay(1000);
      Serial.print("init error\r\n");
  }  
  Serial.println("gprs init success");
  Serial.println("start to send message ...");
  gprsTest.sendSMS(PHONE_NUMBER,MESSAGE); //define phone number and text
}

void loop() {
  //nothing to do
}
