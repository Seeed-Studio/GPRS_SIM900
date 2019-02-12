#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>

#include <avr/pgmspace.h>

#define PIN_TX    7
#define PIN_RX    8

//make sure that the baud rate of SIM900 is 9600!
//you can use the AT Command(AT+IPR=9600) to set it through SerialDebug
#define BAUDRATE  9600

#define RETRY_COUNT 5

GPRS gprs(PIN_TX, PIN_RX, BAUDRATE);

//TODO adjust this
const char apn[] PROGMEM = {"internet.telekom"};

char responseBuffer[32];

void setup(){
  int i;

  Serial.begin(9600);

  Serial.println(F("Start HTTP demonstration, yeah!"));
  Serial.println(F("if something is failing you may want to debug, check sim900.h\r\n"));

  for (i = 0; i < RETRY_COUNT; i++)
  {
      Serial.println(F("Initializing ..."));
      if (gprs.init() != false)
          break;

      delay(500);
  }

  if (i < RETRY_COUNT)
      Serial.println(F("OK"));
  else
  {
      Serial.println(F("failed"));
      return;
  }

  for (i = 0; i < RETRY_COUNT; i++)
  {
      Serial.println(F("Check if network is registered..."));
      if (gprs.isNetworkRegistered() != false)
          break;

      delay(1000);
  }

  if (i < RETRY_COUNT)
      Serial.println(F("OK"));
  else
  {
      Serial.println(F("failed"));
      return;
  }

  Serial.println(F("opening bearer ..."));
  if (gprs.openBearer((const __FlashStringHelper *)apn) != false)
  {
      Serial.print(F("OK, ip address is: "));
      Serial.println(gprs.getIPAddress());
  }
  else
  {
      Serial.println(F("failed"));
      return;
  }

  Serial.println(F("initializing HTTP service ..."));
  if (gprs.httpInitialize() != false)
      Serial.println(F("OK"));
  else
  {
      Serial.println(F("failed"));
      return;
  }

  for (i = 0; i < RETRY_COUNT; i++)
  {
      Serial.println(F("sending HTTP GET request to http://m2msupport.net/m2msupport/test.php ..."));
      if (gprs.httpSendGetRequest(F("http://m2msupport.net/m2msupport/test.php")) != -1)
          break;
  }

  if (i < RETRY_COUNT)
      Serial.println(F("OK"));
  else
  {
      Serial.println(F("failed"));
      return;
  }

  Serial.println(F("fetching HTTP GET response ..."));
  if (gprs.httpReadResponseData(responseBuffer, sizeof(responseBuffer)) != false)
  {
      Serial.println(F("OK, received data:"));
      Serial.println(responseBuffer);
  }
  else
  {
      Serial.println(F("failed"));
      return;
  }

  Serial.println(F("terminating HTTP service ..."));
  if (gprs.httpTerminate() != false)
     Serial.println(F("OK"));
  else
  {
      Serial.println(F("failed"));
      return;
  }

  Serial.println(F("closing bearer ..."));
  if (gprs.closeBearer() != false)
      Serial.println(F("OK"));
  else
  {
      Serial.println(F("failed"));
      return;
  }
}

void loop(){

}
