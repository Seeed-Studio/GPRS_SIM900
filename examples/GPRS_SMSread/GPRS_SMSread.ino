/*
GPRS SMS Read

This sketch is used to test seeeduino GPRS_Shield's reading SMS 
function.To make it work, you should insert SIM card
to Seeeduino GPRS Shield,enjoy it!

There are two methods to read SMS:
  1. GPRS_LoopHandle.ino  -> in order to recieve "+CMTI: "SM"" 
      may be you need to send this command to your shield: "AT+CNMI=2,2,0,0,0"
  2. GPRS_SMSread.ino -> you have to check if there are any 
      UNREAD sms, and you don't need to check serial data continuosly

create on 2015/05/14, version: 1.0
by op2op2op2(op2op2op2@hotmail.com)
*/

#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#define PIN_TX    7
#define PIN_RX    8
#define BAUDRATE  9600

#define MESSAGE_LENGTH 160
char message[MESSAGE_LENGTH];
int messageIndex = 0;

char phone[16];
char datetime[24];

GPRS gprsTest(PIN_TX,PIN_RX,BAUDRATE);//RX,TX,PWR,BaudRate

void setup() {
  Serial.begin(9600);
  while(!gprsTest.init()) {
      Serial.print("init error\r\n");
      delay(1000);
  }
  delay(3000);  
  Serial.println("Init Success, please send SMS message to me!");
}

void loop() {
   messageIndex = gprsTest.isSMSunread();
   if (messageIndex > 0) { //At least, there is one UNREAD SMS
      gprsTest.readSMS(messageIndex, message, MESSAGE_LENGTH, phone, datetime);           
      //In order not to full SIM Memory, is better to delete it
      gprsTest.deleteSMS(messageIndex);
      Serial.print("From number: ");
      Serial.println(phone);  
      Serial.print("Datetime: ");
      Serial.println(datetime);        
      Serial.print("Recieved Message: ");
      Serial.println(message);    
   }
}
