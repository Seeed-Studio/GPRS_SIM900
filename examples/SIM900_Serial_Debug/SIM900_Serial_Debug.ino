/*
SIM900 Serial Debug 

This sketch is used to communicate with SIM900 with AT commands.

create on 2015/05/14, version: 1.0
by lawliet.zou(lawliet.zou@gmail.com)
*/

#include <sim900.h>
#include <SoftwareSerial.h>
#include <Wire.h>

const int pin_tx = 7;
const int pin_rx = 8;

SoftwareSerial gprs(pin_tx,pin_rx);//TX,RX

void setup(){
  Serial.begin(9600);
  sim900_init(&gprs, 9600);
}

void loop(){
  if(gprs.available()){
    Serial.write(gprs.read());
  }
  if(Serial.available()){     
    gprs.write(Serial.read()); 
  }
}



