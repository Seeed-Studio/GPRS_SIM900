/*
GPRS Loop Handle

This sketch is used to test seeeduino GPRS_Shield's call answering and 
reading SMS function.To make it work, you should insert SIM card
to Seeeduino GPRS Shield,enjoy it!

create on 2015/05/14, version: 1.0
by lawliet.zou(lawliet.zou@gmail.com)
*/
#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#define PIN_TX    7
#define PIN_RX    8
#define BAUDRATE  9600

#define MESSAGE_LENGTH 20

char gprsBuffer[64];
int i = 0;
char *s = NULL;
int inComing = 0;

GPRS gprsTest(PIN_TX,PIN_RX,BAUDRATE);

void setup() {
  Serial.begin(9600);
  while(!gprsTest.init()) {
      Serial.print("init error\r\n");
      delay(1000);
  }
  delay(3000);  
  Serial.println("Init Success, please call or send SMS message to me!");
}

void loop() {
   if(gprsTest.readable()) {
       inComing = 1;
   }else{ delay(100);}
   
   if(inComing){
      sim900_read_buffer(gprsBuffer,32,DEFAULT_TIMEOUT);
      //Serial.print(gprsBuffer);
      
      if(NULL != strstr(gprsBuffer,"RING")) {
          gprsTest.answer();
      }else if(NULL != (s = strstr(gprsBuffer,"+CMTI: \"SM\""))) { //SMS: $$+CMTI: "SM",24$$
          char message[MESSAGE_LENGTH];
          int messageIndex = atoi(s+12);
          gprsTest.readSMS(messageIndex, message,MESSAGE_LENGTH);
          Serial.print("Recv Message: ");
          Serial.println(message);
     }
     sim900_clean_buffer(gprsBuffer,32);  
     inComing = 0;
   }
}
