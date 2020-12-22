/*
    GPRS_Shield_Arduino.cpp
    A library for SeeedStudio seeeduino GPRS shield

    Copyright (c) 2015 seeed technology inc.
    Website    : www.seeed.cc
    Author     : lawliet zou
    Create Time: April 2015
    Change Log :

    The MIT License (MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include <stdio.h>
#include "GPRS_Shield_Arduino.h"

GPRS* GPRS::inst = NULL;

GPRS::GPRS(uint8_t tx, uint8_t rx, uint32_t baudRate): gprsSerial(tx, rx) {
    inst = this;
    sim900_init(&gprsSerial, baudRate);
}

bool GPRS::init(void) {
    if (!sim900_check_with_cmd(F("AT\r\n"), "OK\r\n", CMD)) {
        return false;
    }

    if (!sim900_check_with_cmd(F("AT+CFUN=1\r\n"), "OK\r\n", CMD)) {
        return false;
    }

    //180822 Set SMS to text mode
    if (!sim900_check_with_cmd(F("AT+CMGF=1\r\n"), "OK\r\n", CMD)) { // Set message mode to ASCII
        return false;
    }
    //delay(500);	It is not necessary, as we have time before next command

    // set New Message Indicator
    if (!sim900_check_with_cmd(F("AT+CNMI=1,1,0,0,0\r\n"), "OK\r\n", CMD)) { // Set message mode to ASCII
        return false;
    }

    if (!checkSIMStatus()) {
        return false;
    }
    return true;
}

bool GPRS::checkPowerUp(void) {
    return sim900_check_with_cmd(F("AT\r\n"), "OK\r\n", CMD);
    //We dont need here any pin, as this library has to work also with SIM800L
}

void GPRS::powerUpDown(uint8_t pin) {
    // power on pulse for SIM900 Shield
    digitalWrite(pin, LOW);
    delay(1000);
    digitalWrite(pin, HIGH);
    delay(2000);
    digitalWrite(pin, LOW);
    delay(3000);
}

void GPRS::powerReset(uint8_t pin) {
    // reset for SIM800L board.
    // RST pin has to be OUTPUT, LOW and with a NPN transistor
    digitalWrite(pin, HIGH);
    delay(1000);
    digitalWrite(pin, LOW);
    delay(3000);
}


bool GPRS::checkSIMStatus(void) {
    char gprsBuffer[32];
    int count = 0;
    sim900_clean_buffer(gprsBuffer, 32);
    while (count < 3) {
        sim900_send_cmd(F("AT+CPIN?\r\n"));
        sim900_read_buffer(gprsBuffer, 32, DEFAULT_TIMEOUT);
        if ((NULL != strstr(gprsBuffer, "+CPIN: READY"))) {
            break;
        }
        count++;
        delay(300);
    }
    if (count == 3) {
        return false;
    }
    return true;
}

bool GPRS::isNetworkRegistered(void) {
    char gprsBuffer[32];
    int count = 0;
    sim900_clean_buffer(gprsBuffer, 32);
    while (count < 3) {
        sim900_send_cmd(F("AT+CREG?\r\n"));
        sim900_read_buffer(gprsBuffer, 32, DEFAULT_TIMEOUT);
        //Check if home network (0,1) ir roaming (0,5) is enabled
        if ((NULL != strstr(gprsBuffer, "+CREG: 0,1")) || (NULL != strstr(gprsBuffer, "+CREG: 0,5"))) {
            break;
        }
        count++;
        delay(300);
    }
    if (count == 3) {
        return false;
    }
    return true;
}


bool GPRS::sendSMS(const char* number, const char* data) {

    //180822 In the init function
    //if(!sim900_check_with_cmd(F("AT+CMGF=1\r\n"), "OK\r\n", CMD)) { // Set message mode to ASCII
    //    return false;
    //}
    //delay(500);

    sim900_flush_serial();
    sim900_send_cmd(F("AT+CMGS=\""));
    sim900_send_cmd(number);
    //sprintf(cmd,"AT+CMGS=\"%s\"\r\n", number);
    //snprintf(cmd, sizeof(cmd),"AT+CMGS=\"%s\"\r\n", number);
    //    if(!sim900_check_with_cmd(cmd,">",CMD)) {
    if (!sim900_check_with_cmd(F("\"\r\n"), ">", CMD)) {
        return false;
    }
    delay(1000);
    sim900_send_cmd(data);
    delay(500);
    sim900_send_End_Mark();
    return sim900_wait_for_resp("OK\r\n", CMD, 20U, 5000U);
}

char GPRS::isSMSunread() {
    char gprsBuffer[48];  //48 is enough to see +CMGL:
    char* s;


    //List of all UNREAD SMS and DON'T change the SMS UNREAD STATUS
    sim900_send_cmd(F("AT+CMGL=\"REC UNREAD\",1\r\n"));
    /*  If you want to change SMS status to READ you will need to send:
          AT+CMGL=\"REC UNREAD\"\r\n
        This command will list all UNREAD SMS and change all of them to READ

        If there is not SMS, response is (30 chars)
         AT+CMGL="REC UNREAD",1  --> 22 + 2
                                 --> 2
         OK                      --> 2 + 2

        If there is SMS, response is like (>64 chars)
         AT+CMGL="REC UNREAD",1
         +CMGL: 9,"REC UNREAD","XXXXXXXXX","","14/10/16,21:40:08+08"
         Here SMS text.
         OK

         or

         AT+CMGL="REC UNREAD",1
         +CMGL: 9,"REC UNREAD","XXXXXXXXX","","14/10/16,21:40:08+08"
         Here SMS text.
         +CMGL: 10,"REC UNREAD","YYYYYYYYY","","14/10/16,21:40:08+08"
         Here second SMS
         OK
    */

    sim900_clean_buffer(gprsBuffer, 31);
    sim900_read_buffer(gprsBuffer, 30, DEFAULT_TIMEOUT);
    //Serial.print("Buffer isSMSunread: ");Serial.println(gprsBuffer);

    if (NULL != (s = strstr(gprsBuffer, "OK"))) {
        //In 30 bytes "doesn't" fit whole +CMGL: response, if recieve only "OK"
        //    means you don't have any UNREAD SMS
        delay(50);
        return 0;
    } else {
        //More buffer to read
        //We are going to flush serial data until OK is recieved
        sim900_wait_for_resp("OK\r\n", CMD);
        //sim900_flush_serial();
        //We have to call command again
        sim900_send_cmd(F("AT+CMGL=\"REC UNREAD\",1\r\n"));
        sim900_clean_buffer(gprsBuffer, 48);
        sim900_read_buffer(gprsBuffer, 47, DEFAULT_TIMEOUT);
        //Serial.print("Buffer isSMSunread 2: ");Serial.println(gprsBuffer);
        if (NULL != (s = strstr(gprsBuffer, "+CMGL:"))) {
            //There is at least one UNREAD SMS, get index/position
            s = strstr(gprsBuffer, ":");
            if (s != NULL) {
                //We are going to flush serial data until OK is recieved
                sim900_wait_for_resp("OK\r\n", CMD);
                return atoi(s + 1);
            }
        } else {
            return -1;

        }
    }
    return -1;
}

bool GPRS::readSMS(int messageIndex, char* message, int length, char* phone, char* datetime) {
    /*  Response is like:
        AT+CMGR=2

        +CMGR: "REC READ","XXXXXXXXXXX","","14/10/09,17:30:17+08"
        SMS text here

        So we need (more or lees), 80 chars plus expected message length in buffer. CAUTION FREE MEMORY
    */

    int i = 0;
    char gprsBuffer[80 + length];
    //char cmd[16];
    char num[4];
    char* p, *p2, *s;

    //180822 In the init function
    //if(!sim900_check_with_cmd(F("AT+CMGF=1\r\n"), "OK\r\n", CMD)) { // Set message mode to ASCII
    //    return false;
    //}
    //delay(1000);

    //sprintf(cmd,"AT+CMGR=%d\r\n",messageIndex);
    //sim900_send_cmd(cmd);
    sim900_send_cmd(F("AT+CMGR="));
    itoa(messageIndex, num, 10);
    sim900_send_cmd(num);
    sim900_send_cmd(F("\r\n"));
    sim900_clean_buffer(gprsBuffer, sizeof(gprsBuffer));
    sim900_read_buffer(gprsBuffer, sizeof(gprsBuffer));

    if (NULL != (s = strstr(gprsBuffer, "+CMGR:"))) {
        // Extract phone number string
        p = strstr(s, ",");
        p2 = p + 2; //We are in the first phone number character
        p = strstr((char*)(p2), "\"");
        if (NULL != p) {
            i = 0;
            while (p2 < p) {
                phone[i++] = *(p2++);
            }
            phone[i] = '\0';
        }
        // Extract date time string
        p = strstr((char*)(p2), ",");
        p2 = p + 1;
        p = strstr((char*)(p2), ",");
        p2 = p + 2; //We are in the first date time character
        p = strstr((char*)(p2), "\"");
        if (NULL != p) {
            i = 0;
            while (p2 < p) {
                datetime[i++] = *(p2++);
            }
            datetime[i] = '\0';
        }
        if (NULL != (s = strstr(s, "\r\n"))) {
            i = 0;
            p = s + 2;
            while ((*p != '\r') && (i < length - 1)) {
                message[i++] = *(p++);
            }
            message[i] = '\0';
        }
        return true;
    }
    return false;
}

bool GPRS::readSMS(int messageIndex, char* message, int length) {
    int i = 0;
    char gprsBuffer[100];
    //char cmd[16];
    char num[4];
    char* p, *s;

    //180822 In the init function
    //if(!sim900_check_with_cmd(F("AT+CMGF=1\r\n"), "OK\r\n", CMD)) { // Set message mode to ASCII
    //    return false;
    //}
    //delay(1000);

    sim900_send_cmd(F("AT+CMGR="));
    itoa(messageIndex, num, 10);
    sim900_send_cmd(num);
    sim900_send_cmd(F("\r\n"));
    //  sprintf(cmd,"AT+CMGR=%d\r\n",messageIndex);
    //    sim900_send_cmd(cmd);
    sim900_clean_buffer(gprsBuffer, sizeof(gprsBuffer));
    sim900_read_buffer(gprsBuffer, sizeof(gprsBuffer), DEFAULT_TIMEOUT);
    if (NULL != (s = strstr(gprsBuffer, "+CMGR:"))) {
        if (NULL != (s = strstr(s, "\r\n"))) {
            p = s + 2;
            while ((*p != '\r') && (i < length - 1)) {
                message[i++] = *(p++);
            }
            message[i] = '\0';
            return true;
        }
    }
    return false;
}

bool GPRS::deleteSMS(int index) {
    //char cmd[16];
    char num[4];
    //sprintf(cmd,"AT+CMGD=%d\r\n",index);
    sim900_send_cmd(F("AT+CMGD="));
    itoa(index, num, 10);
    sim900_send_cmd(num);
    //snprintf(cmd,sizeof(cmd),"AT+CMGD=%d\r\n",index);
    //sim900_send_cmd(cmd);
    //return 0;
    // We have to wait OK response
    //return sim900_check_with_cmd(cmd,"OK\r\n",CMD);
    return sim900_check_with_cmd(F("\r"), "OK\r\n", CMD);
}

bool GPRS::callUp(char* number) {
    //char cmd[24];
    if (!sim900_check_with_cmd(F("AT+COLP=1\r\n"), "OK\r\n", CMD)) {
        return false;
    }
    delay(1000);
    //HACERR quitar SPRINTF para ahorar memoria ???
    //sprintf(cmd,"ATD%s;\r\n", number);
    //sim900_send_cmd(cmd);
    sim900_send_cmd(F("ATD"));
    sim900_send_cmd(number);
    sim900_send_cmd(F(";\r\n"));
    return true;
}

bool GPRS::answer(void) {
    return sim900_check_with_cmd(F("ATA\r\n"), "OK\r\n", CMD);
}

bool GPRS::hangup(void) {
    return sim900_check_with_cmd(F("ATH\r\n"), "OK\r\n", CMD);
}

bool GPRS::disableCLIPring(void) {
    return sim900_check_with_cmd(F("AT+CLIP=0\r\n"), "OK\r\n", CMD);
}

bool GPRS::getSubscriberNumber(char* number) {
    //AT+CNUM								--> 7 + CR = 8
    //+CNUM: "","+628157933874",145,7,4		--> CRLF + 45 + CRLF = 49
    //										-->
    //OK									--> CRLF + 2 + CRLF = 6

    byte i = 0;
    char gprsBuffer[65];
    char* p, *s;
    sim900_flush_serial();
    sim900_send_cmd(F("AT+CNUM\r\n"));
    sim900_clean_buffer(gprsBuffer, 65);
    sim900_read_buffer(gprsBuffer, 65, DEFAULT_TIMEOUT);
    //Serial.print(gprsBuffer);
    if (NULL != (s = strstr(gprsBuffer, "+CNUM:"))) {
        s = strstr((char*)(s), ",");
        s = s + 2;  //We are in the first phone number character
        p = strstr((char*)(s), "\""); //p is last character """
        if (NULL != s) {
            i = 0;
            while (s < p) {
                number[i++] = *(s++);
            }
            number[i] = '\0';
        }
        return true;
    }
    return false;
}

bool GPRS::isCallActive(char* number) {
    char gprsBuffer[64];  //46 is enough to see +CPAS: and CLCC:
    char* p, *s;
    int i = 0;

    sim900_send_cmd(F("AT+CPAS\r\n"));
    /*  Result code:
        0: ready
        2: unknown
        3: ringing
        4: call in progress

        AT+CPAS   --> 7 + 2 = 9 chars
                --> 2 char
        +CPAS: 3  --> 8 + 2 = 10 chars
                --> 2 char
        OK        --> 2 + 2 = 4 chars

        AT+CPAS

        +CPAS: 0

        OK
    */

    sim900_clean_buffer(gprsBuffer, 64);
    sim900_read_string_until(gprsBuffer, 64, "OK", 2);



    if (NULL != (s = strstr(gprsBuffer, "+CPAS:"))) {
        s = s + 7;
        if (*s != '0') {
            //There is something "running" (but number 2 that is unknow)
            if (*s != '2') {
                //3 or 4, let's go to check for the number
                delay(5);
                sim900_send_cmd(F("AT+CLCC\r\n"));
                /*
                    AT+CLCC --> 9

                    +CLCC: 1,1,4,0,0,"656783741",161,""

                    OK

                    Without ringing:
                    AT+CLCC
                    OK
                */

                sim900_clean_buffer(gprsBuffer, 64);
                sim900_read_string_until(gprsBuffer, 64, "OK", 2);

                if (NULL != (s = strstr(gprsBuffer, "+CLCC:"))) {
                    //There is at least one CALL ACTIVE, get number
                    s = strstr((char*)(s), "\"");
                    s = s + 1;  //We are in the first phone number character
                    p = strstr((char*)(s), "\""); //p is last character """
                    if (NULL != s) {
                        i = 0;
                        while (s < p) {
                            number[i++] = *(s++);
                        }
                        number[i] = '\0';
                    }
                } else {
                    return false;
                }
                return true;
            }
        }
    }
    return false;
}

bool GPRS::getDateTime(char* buffer) {
    //If it doesn't work may be for two reasons:
    //		1. Your carrier doesn't give that information
    //		2. You have to configurate the SIM900 IC.
    //			- First with SIM900_Serial_Debug example try this AT command: AT+CLTS?
    //			- If response is 0, then it is disabled.
    //			- Enable it by: AT+CLTS=1
    //			- Now you have to save this config to EEPROM memory of SIM900 IC by: AT&W
    //			- Now, you have to power down and power up again the SIM900
    //			- Try now again: AT+CCLK?
    //			- It should work

    //AT+CCLK?						--> 8 + CR = 9
    //+CCLK: "14/11/13,21:14:41+04"	--> CRLF + 29+ CRLF = 33
    //
    //OK							--> CRLF + 2 + CRLF =  6

    byte i = 0;
    char gprsBuffer[50];
    char* p, *s;
    sim900_flush_serial();
    sim900_send_cmd(F("AT+CCLK?\r"));
    sim900_clean_buffer(gprsBuffer, 50);
    sim900_read_string_until(gprsBuffer, sizeof(gprsBuffer), "OK\r\n");
    if (NULL != (s = strstr(gprsBuffer, "+CCLK:"))) {
        s = strstr((char*)(s), "\"");
        s = s + 1;  //We are in the first phone number character
        p = strstr((char*)(s), "\""); //p is last character """
        if (NULL != s) {
            i = 0;
            while (s < p) {
                buffer[i++] = *(s++);
            }
            buffer[i] = '\0';
        }
        return true;
    }
    return false;
}

bool GPRS::getVcc(char* buffer) {
    //AT+CBC            --> 6 + CR
    //+CBC: 0,100,4241	--> CRLF + 16 + CRLF
    //
    //OK			    --> CRLF + 2 + CRLF

    byte i = 0;
    char gprsBuffer[50];
    char* p, *s;
    sim900_flush_serial();
    sim900_send_cmd(F("AT+CBC\r"));
    sim900_clean_buffer(gprsBuffer, 50);
    sim900_read_buffer(gprsBuffer, 50, DEFAULT_TIMEOUT);
    if (NULL != (s = strstr(gprsBuffer, "+CBC:"))) {
        s = strstr((char*)(s), ",");
        s = s + 1;
        s = strstr((char*)(s), ",");
        s = s + 1; //We are in the first Vcc character
        p = s + 4; //p is last character
        if (NULL != s) {
            i = 0;
            while (s < p) {
                buffer[i++] = *(s++);
            }
            buffer[i] = '\0';
        }
        return true;
    }
    return false;
}



bool GPRS::getSignalStrength(int* buffer) {
    //AT+CSQ						--> 6 + CR = 10
    //+CSQ: <rssi>,<ber>			--> CRLF + 5 + CRLF = 9
    //OK							--> CRLF + 2 + CRLF =  6

    byte i = 0;
    char gprsBuffer[26];
    char* p, *s;
    char buffers[4];
    sim900_flush_serial();
    sim900_send_cmd(F("AT+CSQ\r"));
    sim900_clean_buffer(gprsBuffer, 26);
    sim900_read_buffer(gprsBuffer, 26, DEFAULT_TIMEOUT);
    if (NULL != (s = strstr(gprsBuffer, "+CSQ:"))) {
        s = strstr((char*)(s), " ");
        s = s + 1;  //We are in the first phone number character
        p = strstr((char*)(s), ",");  //p is last character """
        if (NULL != s) {
            i = 0;
            while (s < p) {
                buffers[i++] = *(s++);
            }
            buffers[i] = '\0';
        }
        *buffer = atoi(buffers);
        return true;
    }
    return false;
}

bool GPRS::sendUSSDSynchronous(char* ussdCommand, char* resultcode, char* response) {
    //AT+CUSD=1,"{command}"
    //OK
    //
    //+CUSD:1,"{response}",{int}

    byte i = 0;
    char gprsBuffer[200];
    char* p, *s;
    sim900_clean_buffer(response, sizeof(response));

    sim900_flush_serial();
    sim900_send_cmd(F("AT+CUSD=1,\""));
    sim900_send_cmd(ussdCommand);
    sim900_send_cmd(F("\"\r"));
    if (!sim900_wait_for_resp("OK\r\n", CMD)) {
        return false;
    }
    sim900_clean_buffer(gprsBuffer, 200);
    sim900_read_buffer(gprsBuffer, 200, DEFAULT_TIMEOUT);
    if (NULL != (s = strstr(gprsBuffer, "+CUSD: "))) {
        *resultcode = *(s + 7);
        resultcode[1] = '\0';
        if (!('0' <= *resultcode && *resultcode <= '2')) {
            return false;
        }
        s = strstr(s, "\"");
        s = s + 1;  //We are in the first phone number character
        p = strstr(s, "\""); //p is last character """
        if (NULL != s) {
            i = 0;
            while (s < p) {
                response[i++] = *(s++);
            }
            response[i] = '\0';
        }
        return true;
    }
    return false;
}

bool GPRS::cancelUSSDSession(void) {
    return sim900_check_with_cmd(F("AT+CUSD=2\r\n"), "OK\r\n", CMD);
}

bool GPRS::getBookEntry(int index, char* number, int* type, char* name) {

    //AT+GPBR=? 		=>		+CPBR:(1-250),40,17		+CPBR: (range phone book),number length, name length
    static const int numMax		= 40;
    static const int nameMax	= 17;

    char gprsBuffer[100];
    char tmp[4];

    sim900_send_cmd(F("AT+CPBR="));
    itoa(index, tmp, 10);
    sim900_send_cmd(tmp);
    sim900_send_cmd(F("\r\n"));

    sim900_clean_buffer(gprsBuffer, sizeof(gprsBuffer));
    sim900_read_buffer(gprsBuffer, sizeof(gprsBuffer), DEFAULT_TIMEOUT);

    //+CPBR: <index>,<number>,<type>,<text>

    char* beg, *end, *idx, *num, *typ;

    if (NULL == (beg = strstr(gprsBuffer, "+CPBR: "))) {
        return false;
    }

    if (NULL == (end = strstr(beg, "\r\n"))) {
        return false;
    }

    if (NULL == (idx = strchr(beg, ','))) {
        return false;
    }

    strncpy(tmp, beg + 7, (idx - (beg + 7)) <= 3 ? (idx - (beg + 7)) : 3);
    tmp[(idx - (beg + 7)) <= 3 ? (idx - (beg + 7)) : 3] = '\0';

    if (strtol(tmp, NULL, 10) != index) {
        return false;
    }

    if (NULL == (num = strchr(idx + 1, ','))) {
        return false;
    }

    strncpy(number, idx + 2, (num - (idx + 3)) <= numMax ? (num - (idx + 3)) :
            numMax);	// We also remove " from the number so we increment idx
    number[(num - (idx + 3)) <= numMax ? (num - (idx + 3)) : numMax] = '\0';

    if (NULL == (typ = strchr(num + 1, ','))) {
        return false;
    }

    strncpy(tmp, num + 1, (typ - (num + 1)) <= 3 ? (typ - (num + 1)) : 3);
    tmp[(typ - (num + 1)) <= 3 ? (typ - (num + 1)) : 3] = '\0';

    if (strtol(tmp, NULL, 10) == 0) {
        return false;
    } else {
        *type = strtol(tmp, NULL, 10);
    }

    strncpy(name, typ + 2, (end - (typ + 3)) <= nameMax ? (end - (typ + 3)) :
            nameMax);		// We also remove " from the name so we increment typ
    name[(end - (typ + 3)) <= nameMax ? (end - (typ + 3)) : nameMax] = '\0';
    return true;
}
bool GPRS::delBookEntry(int index) {
    char indexStr[4];
    itoa(index, indexStr, 10);
    sim900_flush_serial();
    sim900_send_cmd(F("AT+CPBW="));
    sim900_send_cmd(indexStr);
    return sim900_check_with_cmd(F("\r"), "OK\r\n", CMD);
}
bool GPRS::addBookEntry(int index, const char* number, int type, const char* name) {
    char num[4];
    sim900_send_cmd(F("AT+CPBW="));
    if (index != -1) {
        itoa(index, num, 10);
        sim900_send_cmd(num);
    }
    sim900_send_cmd(F(",\""));
    sim900_send_cmd(number);
    sim900_send_cmd(F("\","));
    itoa(type, num, 10);
    sim900_send_cmd(num);
    sim900_send_cmd(F(",\""));
    sim900_send_cmd(name);
    sim900_send_cmd(F("\""));
    return sim900_check_with_cmd(F("\r"), "OK\r\n", CMD);
}

bool GPRS::sleep(void) {
    return sim900_check_with_cmd(F("AT+CSCLK=2\r\n"), "OK\r\n", CMD);
}

bool GPRS::wake(void) {
    unsigned long timerStart;
    bool ok = false;
    //First, send AT dummy command to wake up
    sim900_send_cmd(F("AT\r\n"));
    //Second delay almost 100 ms
    delay(200);
    sim900_flush_serial();
    //Third, send the second AT command to check that it is alive
    timerStart = millis();
    while (!ok && ((unsigned long)(millis() - timerStart) < 3000UL)) {   //Until 3 seconds maximum
        ok = sim900_check_with_cmd(F("AT\r\n"), "OK", CMD);
        delay(300);
    }
    if (ok) {
        //Four, exit sleep mode
        return sim900_check_with_cmd(F("AT+CSCLK=0\r\n"), "OK", CMD);
    } else {
        return false;
    }
}

//Here is where we ask for APN configuration, with F() so we can save MEMORY
bool GPRS::join(const __FlashStringHelper* apn, const __FlashStringHelper* userName,
                const __FlashStringHelper* passWord) {
    byte i;
    char* p, *s;
    char ipAddr[32];
    //Select multiple connection
    //sim900_check_with_cmd("AT+CIPMUX=1\r\n","OK",DEFAULT_TIMEOUT,CMD);

    //set APN. OLD VERSION
    //snprintf(cmd,sizeof(cmd),"AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n",_apn,_userName,_passWord);
    //sim900_check_with_cmd(cmd, "OK\r\n", DEFAULT_TIMEOUT,CMD);

    //It is the user who need to call gprs.close and gprs.disconnect
    //sim900_check_with_cmd("AT+CIPSHUT\r\n", "SHUT OK\r\n", CMD, 2000, 1000);  /**Reset the IP session if any**/

    sim900_send_cmd(F("AT+CSTT=\""));
    if (apn) {
        sim900_send_cmd(apn);
    }
    sim900_send_cmd(F("\",\""));
    if (userName) {
        sim900_send_cmd(userName);
    }
    sim900_send_cmd(F("\",\""));
    if (passWord) {
        sim900_send_cmd(passWord);
    }
    if (!sim900_check_with_cmd(F("\"\r\n"), "OK\r\n", CMD)) {
        return false;
    }


    //Brings up wireless connection
    if (!sim900_check_with_cmd(F("AT+CIICR\r\n"), "OK\r\n", CMD)) {
        return false;
    }

    //Get local IP address
    sim900_send_cmd(F("AT+CIFSR\r\n"));
    sim900_clean_buffer(ipAddr, 32);
    sim900_read_buffer(ipAddr, 32);
    //Response:
    //AT+CIFSR\r\n       -->  8 + 2
    //\r\n				 -->  0 + 2
    //10.160.57.120\r\n  --> 15 + 2 (max)   : TOTAL: 29
    //Response error:
    //AT+CIFSR\r\n
    //\r\n
    //ERROR\r\n
    if (NULL != strstr(ipAddr, "ERROR")) {
        return false;
    }
    s = ipAddr + 11;
    p = strstr((char*)(s), "\r\n"); //p is last character \r\n
    if (NULL != s) {
        i = 0;
        while (s < p) {
            ip_string[i++] = *(s++);
        }
        ip_string[i] = '\0';
    }
    _ip = str_to_ip(ip_string);
    if (_ip != 0) {
        return true;
    }
    return false;
}

void GPRS::disconnect() {
    sim900_check_with_cmd(F("AT+CIPSHUT\r\n"), "SHUT OK\r\n", CMD);
}

bool GPRS::connect(Protocol ptl, const char* host, int port, int timeout, int chartimeout) {
    //char cmd[64];
    char num[4];
    char resp[96];

    //sim900_clean_buffer(cmd,64);
    if (ptl == TCP) {
        sim900_send_cmd(F("AT+CIPSTART=\"TCP\",\""));
        sim900_send_cmd(host);
        sim900_send_cmd(F("\","));
        itoa(port, num, 10);
        sim900_send_cmd(num);
        sim900_send_cmd(F("\r\n"));
        //        sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",host, port);
    } else if (ptl == UDP) {
        sim900_send_cmd(F("AT+CIPSTART=\"UDP\",\""));
        sim900_send_cmd(host);
        sim900_send_cmd(F("\","));
        itoa(port, num, 10);
        sim900_send_cmd(num);
        sim900_send_cmd(F("\r\n"));

        //        sprintf(cmd, "AT+CIPSTART=\"UDP\",\"%s\",%d\r\n",host, port);
    } else {
        return false;
    }


    //sim900_send_cmd(cmd);
    sim900_read_buffer(resp, 96, timeout, chartimeout);
    //Serial.print("Connect resp: "); Serial.println(resp);
    if (NULL != strstr(resp, "CONNECT")) { //ALREADY CONNECT or CONNECT OK
        return true;
    }
    return false;
}

//Overload with F() macro to SAVE memory
bool GPRS::connect(Protocol ptl, const __FlashStringHelper* host, const __FlashStringHelper* port, int timeout,
                   int chartimeout) {
    char resp[96];

    if (ptl == TCP) {
        sim900_send_cmd(F("AT+CIPSTART=\"TCP\",\""));   //%s\",%d\r\n",host, port);
    } else if (ptl == UDP) {
        sim900_send_cmd(F("AT+CIPSTART=\"UDP\",\""));   //%s\",%d\r\n",host, port);
    } else {
        return false;
    }
    sim900_send_cmd(host);
    sim900_send_cmd(F("\","));
    sim900_send_cmd(port);
    sim900_send_cmd(F("\r\n"));
    //	Serial.print("Connect: "); Serial.println(cmd);
    sim900_read_buffer(resp, 96, timeout, chartimeout);
    //	Serial.print("Connect resp: "); Serial.println(resp);
    if (NULL != strstr(resp, "CONNECT")) { //ALREADY CONNECT or CONNECT OK
        return true;
    }
    return false;
}

bool GPRS::is_connected(void) {
    // the largest possible value here is:
    // OK\r\n
    // STATE: SERVER LISTENING\r\n
    // 40 byte should be fine
    char resp[40];
    sim900_send_cmd(F("AT+CIPSTATUS\r\n"));
    sim900_read_buffer(resp, sizeof(resp), DEFAULT_TIMEOUT);
    if (NULL != strstr(resp, "STATE: CONNECT OK")) {
        // OK\r\nSTATE: CONNECT OK
        return true;
    } else {
        // e.g:
        // OK\r\nSTATE: TCP CLOSED
        return false;
    }
}

bool GPRS::close() {
    // if not connected, return
    if (!is_connected()) {
        return true;
    }
    return sim900_check_with_cmd(F("AT+CIPCLOSE\r\n"), "CLOSE OK\r\n", CMD);
}

int GPRS::readable(void) {
    return sim900_check_readable();
}

int GPRS::wait_readable(int wait_time) {
    return sim900_wait_readable(wait_time);
}

int GPRS::wait_writeable(int req_size) {
    return req_size + 1;
}

int GPRS::send(const char* str, int len) {
    //char cmd[32];
    char num[4];
    if (len > 0) {
        //snprintf(cmd,sizeof(cmd),"AT+CIPSEND=%d\r\n",len);
        //sprintf(cmd,"AT+CIPSEND=%d\r\n",len);
        sim900_send_cmd(F("AT+CIPSEND="));
        itoa(len, num, 10);
        sim900_send_cmd(num);
        if (!sim900_check_with_cmd(F("\r\n"), ">", CMD)) {
            //if(!sim900_check_with_cmd(cmd,">",CMD)) {
            return 0;
        }
        /*  if(0 != sim900_check_with_cmd(str,"SEND OK\r\n", DEFAULT_TIMEOUT * 10 ,DATA)) {
            return 0;
            }*/
        delay(500);
        sim900_send_cmd(str);
        delay(500);
        sim900_send_End_Mark();
        if (!sim900_wait_for_resp("SEND OK\r\n", DATA, DEFAULT_TIMEOUT * 10, DEFAULT_INTERCHAR_TIMEOUT * 10)) {
            return 0;
        }
    }
    return len;
}

boolean GPRS::send(const __FlashStringHelper* str) {
    if (!sim900_check_with_cmd(F("AT+CIPSEND\r\n"), ">", CMD)) {
        return false;
    }

    sim900_send_cmd(str);
    sim900_send_End_Mark();

    if (!sim900_wait_for_resp("SEND OK\r\n", DATA, DEFAULT_TIMEOUT, DEFAULT_INTERCHAR_TIMEOUT)) {
        return false;
    }

    return true;
}

boolean GPRS::send(const char* str) {
    if (!sim900_check_with_cmd(F("AT+CIPSEND\r\n"), ">", CMD)) {
        return false;
    }
    delay(500);
    sim900_send_cmd(str);
    delay(500);
    sim900_send_End_Mark();
    if (!sim900_wait_for_resp("SEND OK\r\n", DATA, DEFAULT_TIMEOUT * 10, DEFAULT_INTERCHAR_TIMEOUT * 10)) {
        return false;
    }
    return true;
}


int GPRS::recv(char* buf, int len) {
    sim900_clean_buffer(buf, len);
    sim900_read_buffer(buf,
                       len);  //Ya he llamado a la funcion con la longitud del buffer - 1 y luego le estoy añadiendo el 0
    return strlen(buf);
}

void GPRS::listen(void) {
    gprsSerial.listen();
}

bool GPRS::isListening(void) {
    return gprsSerial.isListening();
}



uint32_t GPRS::str_to_ip(const char* str) {
    uint32_t ip = 0;
    char* p = (char*)str;
    for (int i = 0; i < 4; i++) {
        ip |= atoi(p);
        p = strchr(p, '.');
        if (p == NULL) {
            break;
        }
        ip <<= 8;
        p++;
    }
    return ip;
}

char* GPRS::getIPAddress() {
    //I have already a buffer with ip_string: snprintf(ip_string, sizeof(ip_string), "%d.%d.%d.%d", (_ip>>24)&0xff,(_ip>>16)&0xff,(_ip>>8)&0xff,_ip&0xff);
    return ip_string;
}

unsigned long GPRS::getIPnumber() {
    return _ip;
}
/*  NOT USED bool GPRS::gethostbyname(const char* host, uint32_t* ip)
    {
    uint32_t addr = str_to_ip(host);
    char buf[17];
    //snprintf(buf, sizeof(buf), "%d.%d.%d.%d", (addr>>24)&0xff, (addr>>16)&0xff, (addr>>8)&0xff, addr&0xff);
    if (strcmp(buf, host) == 0) {
         ip = addr;
        return true;
    }
    return false;
    }
*/

bool GPRS::getLocation(const __FlashStringHelper* apn, float* longitude, float* latitude) {
    int i = 0;
    char gprsBuffer[80];
    char buffer[20];
    char* s;

    if (openBearer(apn) == false) {
        return false;
    }

    //AT+CIPGSMLOC=1,1
    sim900_flush_serial();
    sim900_send_cmd(F("AT+CIPGSMLOC=1,1\r"));
    sim900_clean_buffer(gprsBuffer, sizeof(gprsBuffer));
    sim900_read_buffer(gprsBuffer, sizeof(gprsBuffer), 2 * DEFAULT_TIMEOUT, 6 * DEFAULT_INTERCHAR_TIMEOUT);
    //Serial.println(gprsBuffer);

    if (NULL != (s = strstr(gprsBuffer, "+CIPGSMLOC:"))) {
        s = strstr((char*)s, ",");
        s = s + 1;
        //Serial.println(*s);
        i = 0;
        while (*(++s) !=  ',') {
            buffer[i++] = *s;
        }
        buffer[i] = 0;
        *longitude = atof(buffer);

        i = 0;
        while (*(++s) !=  ',') {
            buffer[i++] = *s;
        }
        buffer[i] = 0;
        *latitude = atof(buffer);
        return true;
    }
    return false;
}

bool GPRS::openBearer(const __FlashStringHelper* apn) {
    const uint8_t checkBearerRetryCount = 3;
    int i;

    //send AT+SAPBR=3,1,"Contype","GPRS"
    if (sim900_check_with_cmd(F("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r"), "OK\r\n", CMD) == false) {
        return false;
    }

    //send AT+SAPBR=3,1,"APN","<apn>"
    sim900_send_cmd(F("AT+SAPBR=3,1,\"APN\",\""));
    if (apn) {
        sim900_send_cmd(apn);
    }

    if (sim900_check_with_cmd(F("\"\r"), "OK\r\n", CMD) == false) {
        return false;
    }

    //send AT+SAPBR =1,1
    if (sim900_check_with_cmd(F("AT+SAPBR=1,1\r\n"), "OK\r\n", CMD) == false) {
        return false;
    }

    for (i = 0; i < checkBearerRetryCount; i++) {
        // 3 means "closed"
        uint8_t bearerStatus = 3;
        // 1 means "connected"
        queryBearer(&bearerStatus);
        if (bearerStatus == 1) {
            break;
        }
    }

    if (i >= checkBearerRetryCount) {
        return false;
    }

    return true;
}

bool GPRS::queryBearer(uint8_t* bearerStatus) {
    char receiveBuffer[32];
    char* commaPtr;

    // send AT+SAPBR=2,1 and read "+SAPBR"
    if (sim900_check_with_cmd(F("AT+SAPBR=2,1\r\n"), "+SAPBR:", DATA) == false) {
        return false;
    }

    sim900_clean_buffer(receiveBuffer, sizeof(receiveBuffer));

    // check response which looks like:
    // +SAPBR: <cid>,<Status>,<IP_Addr>\r\nOK

    // read cid (always 1)
    if (sim900_read_string_until(receiveBuffer, sizeof(receiveBuffer), "1,") == NULL) {
        return false;
    }

    // read until next comma -> status
    commaPtr = sim900_read_string_until(receiveBuffer, sizeof(receiveBuffer), ",");
    if (commaPtr == NULL) {
        return false;
    }

    // replace comma with string termination
    *commaPtr = '\0';

    // now extract status
    *bearerStatus = (uint8_t)atol(receiveBuffer);

    // only check for ip address if we are connected (=1)
    if (*bearerStatus == 1) {
        char* endOfIpAddress = NULL;

        // read ip address, which is enclosed in '"', so read first '"'
        if (sim900_read_string_until(receiveBuffer, sizeof(receiveBuffer), "\"") == NULL) {
            return false;
        }

        // read second '"'
        endOfIpAddress = sim900_read_string_until(receiveBuffer, sizeof(receiveBuffer), "\"");
        if (endOfIpAddress == NULL) {
            return false;
        }

        *endOfIpAddress = '\0';

        strncpy(ip_string, receiveBuffer, sizeof(ip_string));
        _ip = str_to_ip(ip_string);
    }

    // flush rest of data which should be "\r\nOK"
    sim900_flush_serial();

    return true;;
}

bool GPRS::closeBearer(void) {
    //TODO maybe also call queryBearer() here and check if it really was closed (as in openBearer)
    return sim900_check_with_cmd(F("AT+SAPBR=0,1\r\n"), "OK\r\n", CMD);
}

bool GPRS::ntpSyncDateTime(int8_t timezone) {
    // e.g. "-105\0", this is not a valid value to pass to the SIM module, but we need to make sure not
    // to cross boundaries of the char array
    char timezoneString[5] = {'\0'};

    if (sim900_check_with_cmd(F("AT+CNTPCID=1\r\n"), "OK\r\n", CMD) == false) {
        return false;
    }

    sim900_send_cmd(F("AT+CNTP=\"pool.ntp.org\","));

    itoa(timezone, timezoneString, 10);

    sim900_send_cmd(timezoneString);

    if (sim900_check_with_cmd(F("\r\n"), "OK\r\n", CMD) == false) {
        return false;
    }

    return sim900_check_with_cmd(F("AT+CNTP\r\n"), "+CNTP: 1", CMD);
}

bool GPRS::httpInitialize(void) {
    return sim900_check_with_cmd(F("AT+HTTPINIT\r\n"), "OK", CMD);
}

bool GPRS::httpTerminate(void) {
    return sim900_check_with_cmd(F("AT+HTTPTERM\r\n"), "OK", CMD);
}

int16_t GPRS::httpSendGetRequest(const __FlashStringHelper* url, const __FlashStringHelper* path, uint16_t port) {
    return httpSendGetRequest(url, "", path, 0, NULL, NULL, port);
}

int16_t GPRS::httpSendGetRequest(const __FlashStringHelper* url,
                                 const char* pathPart1,
                                 const __FlashStringHelper* pathPart2,
                                 uint8_t queryParametersCount,
                                 const __FlashStringHelper* const queryParameterKeys[],
                                 const char* const queryParamValues[],
                                 uint16_t port) {
    char receiveBuffer[32];
    char httpStatusCode[4];
    char* commaPtr = NULL;
    const __FlashStringHelper* tempParamKey;

    // 1 AT+HTTPPARA=\"CID\",1
    if (sim900_check_with_cmd(F("AT+HTTPPARA=\"CID\",1\r\n"), "OK", CMD) == false) {
        return -1;
    }

    // 2 AT+HTTPPARA=\"URL\",\"<url>\"
    sim900_send_cmd(F("AT+HTTPPARA=\"URL\",\""));

    sim900_send_cmd(url);

    if (port != HTTP_DEFAULT_PORT) {
        char strPort[6] = {'\0'};
        ltoa(port, strPort, 10);
        sim900_send_cmd(F(":"));
        sim900_send_cmd(strPort);
    }

    sim900_send_cmd(pathPart1);
    sim900_send_cmd(pathPart2);

    for (int i = 0; i < queryParametersCount; i++) {
        // first parameter
        if (i == 0) {
            sim900_send_cmd("?");
        } else {
            sim900_send_cmd("&");
        }

        // copies the pointers to the strings from progmem to RAM
        // got this progmem magic from http://www.gammon.com.au/progmem
        tempParamKey = (const __FlashStringHelper*)pgm_read_word(&queryParameterKeys[i]);

        sim900_send_cmd(tempParamKey);
        sim900_send_cmd("=");
        sim900_send_cmd(queryParamValues[i]);
    }

    if (sim900_check_with_cmd(F("\"\r\n"), "OK", CMD) == false) {
        return -1;
    }

    // 3 AT+HTTPACTION=0
    if (sim900_check_with_cmd(F("AT+HTTPACTION=0\r\n"), "OK", DATA) == false) {
        return -1;
    }

    // fetch additional data which looks like +HTTPACTION: <Method>,<StatusCode>,<DataLen>
    // where "Method" is always 0 (GET request)
    sim900_clean_buffer(receiveBuffer, sizeof(receiveBuffer));
    if (sim900_read_string_until(receiveBuffer, sizeof(receiveBuffer), "+HTTPACTION: 0,", 15, 15000) == NULL) {
        return -1;
    }

    sim900_clean_buffer(receiveBuffer, sizeof(receiveBuffer));
    // let's not waste time: request should be finished after 1000ms
    sim900_read_buffer(receiveBuffer, sizeof(receiveBuffer), 1, 1000);

    sim900_clean_buffer(httpStatusCode, sizeof(httpStatusCode));
    //status codes are always 3 chars
    strncpy(httpStatusCode, receiveBuffer, 3);

    if (strcmp(httpStatusCode, "200") != 0) {
        return -1;
    }

    // search for additional comma, DataLen comes after that
    commaPtr = strrchr(receiveBuffer, ',');
    if (commaPtr == NULL) {
        return -1;
    }

    // step over comma
    commaPtr++;

    return atol(commaPtr);
}

bool GPRS::httpReadResponseData(char* buffer, uint16_t bufferSize) {
    char receiveBuffer[32];
    char* charFoundPtr = NULL;

    // issue command
    if (sim900_check_with_cmd(F("AT+HTTPREAD\r\n"), "+HTTPREAD:", DATA) == false) {
        return false;
    }

    sim900_clean_buffer(receiveBuffer, sizeof(receiveBuffer));

    // analyzing response which looks like:
    //+HTTPREAD:<data_len>
    //<data>
    //OK

    // read first line of response, we ignore the data length as httpSendGetRequest already
    // returns this information
    if (sim900_read_string_until(receiveBuffer, sizeof(receiveBuffer), "\r\n") == false) {
        return false;
    }

    sim900_clean_buffer(buffer, bufferSize);

    // read HTTP response data and the following "OK"
    // Note: at this point the response was already received (via GPRS), this is just
    // reading the data from the module. This should be done in 500ms
    charFoundPtr = sim900_read_string_until(buffer, bufferSize, "\r\nOK", 1, 500);

    if (charFoundPtr == NULL) {
        return false;
    }

    // write \0 to index where the pattern was found to terminate the received HTTP response data
    *charFoundPtr = '\0';

    sim900_flush_serial();

    return true;
}

void GPRS::AT_Bypass() {
    sim900_AT_bypass();
}
