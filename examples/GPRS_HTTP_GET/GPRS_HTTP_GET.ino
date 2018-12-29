#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>

#include <avr/pgmspace.h>

#define PIN_TX    7
#define PIN_RX    8

//make sure that the baud rate of SIM900 is 9600!
//you can use the AT Command(AT+IPR=9600) to set it through SerialDebug
#define BAUDRATE  9600

GPRS gprs(PIN_TX, PIN_RX, BAUDRATE);

char responseBuffer[32];

void setup(){
  Serial.begin(9600);

  Serial.println(F("Start HTTP demonstration..."));

  while(!gprs.init()) {
      delay(500);
      Serial.println(F("Initializing..."));
  }

  while(!gprs.isNetworkRegistered())
  {
    delay(1000);
    Serial.println(F("Network has not registered yet!"));
  }

  Serial.println(F("opening bearer ..."));
  if (gprs.openBearer(F("internet.telekom")) != false)
  {
      Serial.println(F("opening bearer OK"));
  }

  Serial.println(F("initializing HTTP service ..."));
  if (gprs.httpInitialize() != false)
  {
      Serial.println(F("HTTP init OK"));
  }

  for (int i = 0; i < 3; i++)
  {
      Serial.println(F("sending HTTP GET request ..."));
      if (gprs.httpSendGetRequest(F("http://m2msupport.net/m2msupport/test.php")) != -1)
      {
          Serial.println(F("sending HTTP GET request OK"));
          break;
      }
  }

  Serial.println(F("fetching HTTP GET response ..."));
  if (gprs.httpReadResponseData(responseBuffer, sizeof(responseBuffer)) != false)
  {
      Serial.println(F("fetching HTTP GET response OK, received data:"));
      Serial.println(responseBuffer);
  }

  Serial.println(F("terminating HTTP service ..."));
  if (gprs.httpTerminate() != false)
  {
     Serial.println(F("HTTP terminate OK"));
  }


  Serial.println(F("closing bearer ..."));
  if (gprs.closeBearer() != false)
  {
      Serial.println(F("closing bearer OK"));
  }
}

void loop(){

}
