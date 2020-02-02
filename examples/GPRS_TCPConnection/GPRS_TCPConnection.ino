
#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#define PIN_TX    7
#define PIN_RX    8
//make sure that the baud rate of SIM900 is 9600!
//you can use the AT Command(AT+IPR=9600) to set it through SerialDebug
#define BAUDRATE  9600

char http_cmd[] = "GET /media/uploads/mbed_official/hello.txt HTTP/1.0\r\n\r\n";
char buffer[512];
GPRS gprs(PIN_TX, PIN_RX, BAUDRATE);
void setup() {
    gprs.checkPowerUp();
    Serial.begin(9600);
    Serial.print("Start TCP demostration...\r\n");

    // use DHCP
    while (!gprs.init()) {
        delay(1000);
        Serial.print("Initializing...\r\n");
    }

    while (!gprs.isNetworkRegistered()) {
        delay(1000);
        Serial.println("Network has not registered yet!");
    }

    delay(3000);
    // attempt DHCP
    while (!gprs.join(F("cmnet"))) {
        Serial.println("gprs join network error");
        delay(2000);
    }

    // successful DHCP
    Serial.print("IP Address is ");
    Serial.println(gprs.getIPAddress());

    if (!gprs.connect(TCP, "mbed.org", 80)) {
        Serial.println("connect error");
    } else {
        Serial.println("connect mbed.org success");
    }

    Serial.println("waiting to fetch...");
    gprs.send(http_cmd, sizeof(http_cmd) - 1);
    while (true) {
        int ret = gprs.recv(buffer, sizeof(buffer) - 1);
        if (ret <= 0) {
            Serial.println("fetch over...");
            break;
        }
        buffer[ret] = '\0';
        Serial.print("Recv: ");
        Serial.print(ret);
        Serial.print(" bytes: ");
        Serial.println(buffer);
    }
    gprs.close();
    gprs.disconnect();
}

void loop() {

}
