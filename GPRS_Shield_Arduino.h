/*
    GPRS_Shield_Arduino.h
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

#ifndef __GPRS_SHIELD_ARDUINO_H__
#define __GPRS_SHIELD_ARDUINO_H__

#include "sim900.h"

/** GPRS class.
    used to realize GPRS communication
*/

#define HTTP_DEFAULT_PORT 80u

enum Protocol {
    CLOSED = 0,
    TCP    = 1,
    UDP    = 2,
};

class GPRS {
  public:
    /** Create GPRS instance
        @param number default phone number during mobile communication
    */
    GPRS(uint8_t tx,  uint8_t rx, uint32_t baudRate = 9600);

    /** get instance of GPRS class
    */
    static GPRS* getInstance() {
        return inst;
    };

    /** initialize GPRS module including SIM card check & signal strength
        @return true if connected, false otherwise
    */
    bool init(void);


    /** check if GPRS module is powered on or not
        @returns
            true on success
            false on error
    */
    bool checkPowerUp(void);


    /** power Up GPRS module (JP has to be soldered)
        @param  pin pin 9 connected to JP jumper so we can power up and down through software
        @returns

    */
    void powerUpDown(uint8_t pin = 9);

    /** power reset for SIM800 board
        @param  pin (preconfigurated as OUTPUT)
        @returns

    */
    void powerReset(uint8_t pin);
    /** Check network registration status
        @return true on success, false on fail
    */
    bool isNetworkRegistered(void);

    /** Check SIM card status
        @return
              true for SIM inserted
              false for no SIM detected
    */
    bool checkSIMStatus(void);

    /** send text SMS
        @param  *number phone number which SMS will be send to
        @param  *data   message that will be send to
        @returns
            true on success
            false on error
    */
    bool sendSMS(const char* number, const char* data);

    /** Check if there is any UNREAD SMS: this function DOESN'T change the UNREAD status of the SMS
        @returns
            1..20 on success, position/index where SMS is stored, suitable for the function ReadSMS
            -1 on error
             0 - there is no SMS with specified status (UNREAD)
    */
    char isSMSunread();

    /** read SMS, phone and date if getting a SMS message. It changes SMS status to READ
        @param  messageIndex  SIM position to read
        @param  message  buffer used to get SMS message
        @param  length  length of message buffer
        @param  phone  buffer used to get SMS's sender phone number
        @param  datetime  buffer used to get SMS's send datetime
        @returns
            true on success
            false on error
    */
    bool readSMS(int messageIndex, char* message, int length, char* phone, char* datetime);

    /** read SMS if getting a SMS message
        @param  buffer  buffer that get from GPRS module(when getting a SMS, GPRS module will return a buffer array)
        @param  message buffer used to get SMS message
        @param  check   whether to check phone number(we may only want to read SMS from specified phone number)
        @returns
            true on success
            false on error
    */
    bool readSMS(int messageIndex, char* message, int length);

    /** delete SMS message on SIM card
        @param  index   the index number which SMS message will be delete
        @returns
            true on success
            false on error
    */
    bool deleteSMS(int index);

    /** call someone
        @param  number  the phone number which you want to call
        @returns
            true on success
            false on error
    */
    bool callUp(char* number);

    /** auto answer if coming a call
        @returns
            true on success
            false on error
    */
    bool answer(void);

    /** hang up if coming a call
        @returns
            true on success
            false on error
    */
    bool hangup(void);

    /** Disable +CLIP notification when an incoming call is active, RING text is always shown. See isCallActive function
        This is done in order no to overload serial outputCheck if there is a call active and get the phone number in that case
        @returns
            true on success
            false on error
    */
    bool disableCLIPring(void);

    /** Get Subscriber Number (your number) using AT+CNUM command, but if nothing returns, then
        you need to command this to your SIM900. (See AT+CPBS, AT+CPBW)
     	AT+CPBS="ON"
     	AT+CPBW=1,"+{Your Number}",145
     	AT+CPBS="SM"
        @param
     	@return
     		true on success
     		false on error
    */
    bool getSubscriberNumber(char* number);

    /** Check if there is a call active and get the phone number in that case
        @returns
            true on success
            false on error
    */
    bool isCallActive(char* number);

    /** get DateTime from SIM900 (see AT command: AT+CLTS=1) as string
        @param
        @returns
            true on success
            false on error

      	   If it doesn't work may be for two reasons:
     	 	1. Your carrier doesn't give that information
     		2. You have to configurate the SIM900 IC.
     			- First with SIM900_Serial_Debug example try this AT command: AT+CLTS?
     			- If response is 0, then it is disabled.
     			- Enable it by: AT+CLTS=1
     			- Now you have to save this config to EEPROM memory of SIM900 IC by: AT&W
     			- Now, you have to power down and power up again the SIM900
     			- Try now again: AT+CCLK?
     			- It should work now

    */
    bool getDateTime(char* buffer);

    /** get battery voltage from SIM900 in mV
        @param
        @returns
            true on success
            false on error
    */
    bool getVcc(char* buffer);

    /** get Signal Strength from SIM900 (see AT command: AT+CSQ) as integer
        @param
        @returns
           true on success
           false on error
    */
    bool getSignalStrength(int* buffer);

    /** Send USSD Command Synchronously (Blocking call until unsolicited response is received)
        @param
     *		*ussdCommand string command UUSD, ex: *123#
     *		*resultCode	char Result Code, see AT+CUSD command
     *		*response	string response
     *		*cellBroadcast	int Cell Broadcast Data Coding Scheme
        @returns
            true on success
            false on error
    */
    bool sendUSSDSynchronous(char* ussdCommand, char* resultcode, char* response);

    /** Cancel USSD Session
        @returns
            true on success cancel active session
            false on error or because no active session
    */
    bool cancelUSSDSession(void);

    /** Get number and name from the phone book
        @param  index	phone book position
        @param  number	buffer used to get the number
        @param	type	buffer used to get the type of the number e.g. national(129/161), international(145), network specific(177)	refer to: http://m2msupport.net/m2msupport/sim-phonebook-at-commands/
        @param  name	buffer used to get the name
        @returns
            true on success
            false on error or entry didn't exist
    */
    bool getBookEntry(int index, char* number, int* type, char* name);

    /** Delete phone book entry
        @param  index	phone book position
        @returns
            true on success or entry didn't exist
            false on error
    */
    bool delBookEntry(int index);

    /** Add number and name to the phone book
        @param  index	phone book position -1 for choosing the next free index
        @param  number	buffer used to add the number
        @param	type	buffer used to add the type of the number e.g. national(129/161), international(145), network specific(177)	refer to: http://m2msupport.net/m2msupport/sim-phonebook-at-commands/
        @param  name	buffer used to add the name
        @returns
            true on success
            false on error
    */
    bool addBookEntry(int index, const char* number, int type, const char* name);

    //////////////////////////////////////////////////////
    /// SLEEP
    //////////////////////////////////////////////////////
    /** Enter sleep mode (AT+CSCLK=2)
        @returns
            true on success
            false on error
    */
    bool sleep(void);

    /** Exit sleep mode (AT+CSCLK=0)
        @returns
            true on success
            false on error
    */
    bool wake(void);

    //////////////////////////////////////////////////////
    /// GPRS
    //////////////////////////////////////////////////////
    /**  Connect the GPRS module to the network.
        @return true if connected, false otherwise
    */
    bool join(const __FlashStringHelper* apn = 0, const __FlashStringHelper* userName = 0,
              const __FlashStringHelper* passWord = 0);

    /** Disconnect the GPRS module from the network
        @returns
    */
    void disconnect(void);

    /** Open a tcp/udp connection with the specified host on the specified port
        @param socket an endpoint of an inter-process communication flow of GPRS module,for SIM900 module, it is in [0,6]
        @param ptl protocol for socket, TCP/UDP can be choosen
        @param host host (can be either an ip address or a name. If a name is provided, a dns request will be established)
        @param port port
        @param timeout wait seconds till connected
        @param chartimeout wait milliseconds between characters from GPRS module
        @returns true if successful
    */
    bool connect(Protocol ptl, const char* host, int port, int timeout = 2 * DEFAULT_TIMEOUT,
                 int chartimeout = 2 * DEFAULT_INTERCHAR_TIMEOUT);
    bool connect(Protocol ptl, const __FlashStringHelper* host, const __FlashStringHelper* port,
                 int timeout = 2 * DEFAULT_TIMEOUT, int chartimeout = 2 * DEFAULT_INTERCHAR_TIMEOUT);

    /** Check if a tcp link is active
        @returns true if successful
    */
    bool is_connected(void);

    /** Close a tcp connection
        @returns true if successful
    */
    bool close(void);

    /** check if GPRS module is readable or not
        @returns true if readable
    */
    int readable(void);

    /** wait a few time to check if GPRS module is readable or not
        @param wait_time time of waiting
    */
    int wait_readable(int wait_time);

    /** wait a few time to check if GPRS module is writeable or not
        @param wait_time time of waiting
    */
    int wait_writeable(int req_size);

    /** send data to socket
        @param str string to be sent
        @param len string length
        @returns return bytes that actually been send
    */
    int send(const char* str, int len);

    /** send data to socket without AT+CIPSEND=len
        @param str string to be sent (from progmem)
        @returns true if successful, false if a timeout occured
    */
    boolean send(const __FlashStringHelper* str);

    /** send data to socket without AT+CIPSEND=len
        @param str string to be sent
        @returns true if successful
    */
    boolean send(const char* str);

    /** read data from socket
        @param buf buffer that will store the data read from socket
        @param len string length need to read from socket
        @returns bytes that actually read
    */
    int recv(char* buf, int len);

    /** Enables the selected software serial port to listen
        @returns none
    */
    void listen(void);

    /** Tests to see if requested software serial port is actively listening.
        @returns none
    */
    bool isListening(void);

    /** convert the host to ip
        @param host host ip string, ex. 10.11.12.13
        @param ip long int ip address, ex. 0x11223344
        @returns true if successful
    */
    //NOT USED bool gethostbyname(const char* host, uint32_t* ip);

    char* getIPAddress();
    unsigned long getIPnumber();
    bool getLocation(const __FlashStringHelper* apn, float* longitude, float* latitude);

    //////////////////////////////////////////////////////
    /// HTTP + NTP
    /// the implementation is based on information from SIM900_AT Command Manual_ V1.03
    /// and SIM800 Series_NTP_Application+Note_V1.01.pdf for NTP
    //////////////////////////////////////////////////////
    /** opens bearer: set parameters, and open it, use closeBearer() to close again
        1 AT+SAPBR=3,1,"Contype","GPRS"
        2 AT+SAPBR=3,1,"APN","<apn>"
        3 AT+SAPBR =1,1
        @param apn: access point name
        TODO: maybe add parameters for user name and password as well, for me it is working without them

        @returns true if successful, false if there was a timeout
    */
    bool openBearer(const __FlashStringHelper* apn);

    /** closes bearer, use openBearer() to open:
        1 AT+SAPBR=0,1
        @returns true if successful, false if there was a timeout
    */
    bool closeBearer(void);

    /** !!!SIM800* only, see Note2 in this comment!!!
        synchronizes the SIM module's RTC using pool.ntp.org, requires openBearer()
        to be called beforehand. The actual time can be retrieved using getDateTime().

        Sends the following commands:
        1 AT+CNTPCID=1
        2 AT+CNTP=\"pool.ntp.org\",<timezone>
        3 AT+CNTP
        @param timezone: timezone is specified in quarters, e.g. 0 = UTC, 8 = +2:00, -38 = -9:30
        according to SIM800 Series_NTP_Application Note_V1.01 the valid range is from -47 to 48.
        The range is not checked but instead directly passed to the module.

        @returns true if successful, false if there was something wrong

        Note1: I'm not sure how this works together with automatic time syncing via
        network operator (AT+CLTS=1). Precautionary I set AT+CLTS=0. See getDateTime() for information regarding this.
        Note2: SIM900 uses another interface, see SIM900 NTP AT Command Manual_V1.00.pdf
        It uses these commands:
        1 AT+CNTP="pool.ntp.org ",8,1,2
        2 AT+CNTP (which then directly returns the time)
        Maybe the device can be queried if it is SIM800* or SIM900*. Then the second implementation can be added
        as well.
    */
    bool ntpSyncDateTime(int8_t timezone);

    /** initializes HTTP service by issuing AT+HTTPINIT command
        @returns true if successful, false if an error occurred
    */
    bool httpInitialize(void);

    /** terminates HTTP service by issuing AT+HTTPTERM command
        @returns true if successful, false if an error occurred
    */
    bool httpTerminate(void);

    /** send HTTP GET request, requires openBearer() to be called beforehand. The parameters
        url and path will we concatenated together, if a port != 80 is specified it will be put in
        between: <url>:<port><path>. The following commands are sent:
        1 AT+HTTPPARA=\"CID\",1
        2 AT+HTTPPARA=\"URL\",\"<url>:<port><path>\"
        3 AT+HTTPACTION=0
        @param url: URL or IP address including "http://", e.g.
                     e.g. "http://m2msupport.net"
        @param port: http port, e.g. 8080
        @param path: path to file or directory, e.g. "/m2msupport/test.php", use F("") to leave empty
        @returns amount of bytes the server returned (may also be 0), -1 indicates a
        an error in executing the AT* commands or if the webserver returned a status
        code != 200 (OK).
        The data may be fetched using httpReadResponseData()
    */
    int16_t httpSendGetRequest(const __FlashStringHelper* url,
                               const __FlashStringHelper* path,
                               uint16_t port = HTTP_DEFAULT_PORT);

    /** send HTTP GET request, requires openBearer() to be called beforehand. The parameters
        url, pathPart1 and pathPart2 will we concatenated together. This allows to save RAM (by putting
        url and pathPart2 in PROGMEM) and to modify the path during runtime if required. Example:
        http:/testserver.ab/<modifiablePart>/test/path
        Parameters to construct this request:
        url = F("http:/testserver.ab")
        pathPart1 = "/<http:/testserver.ab>"
        pathPart2 = F("/test/path")

        If a port != 80 is specified it will be put in between: <url>:<port><pathPart1><pathPart2>.

        Furthermore this method allows to put parameters in a RAM-saving way into the GET request.
        See the HTTP example sketch for how to use it.

        The following commands are sent:
        1 AT+HTTPPARA=\"CID\",1
        2 AT+HTTPPARA=\"URL\",\"<url>:<port><path>\"
        3 AT+HTTPACTION=0
        @param url: URL or IP address including "http://", e.g.
                     e.g. "http://m2msupport.net"
        @param pathPart1: part1 of path to file or directory, e.g. "/m2msupport/", use "" to leave empty.
                          This MUST be \0-terminated
        @param pathPart2: part 2 of path to file or directory, e.g. "test.php", use F("") to leave empty
        @param queryParametersCount: amount of parameters to be added to the GET request, one parameter consists of
                                     a key value pair, may also be 0 to add 0 parameters
        @param queryParameterKeys: the parameter keys, specified by an array of pointers located in PROGMEM
                                   pointing to strings located in PROGMEM, see HTTP example sketch
        @param queryParamValues: the parameter values in an array containing C-style strings

        @param port: http port, e.g. 8080

        @returns amount of bytes the server returned (may also be 0), -1 indicates a
        an error in executing the AT* commands or if the webserver returned a status
        code != 200 (OK).
        The data may be fetched using httpReadResponseData()
    */
    int16_t httpSendGetRequest(const __FlashStringHelper* url,
                               const char* pathPart1,
                               const __FlashStringHelper* pathPart2,
                               uint8_t queryParametersCount,
                               const __FlashStringHelper* const queryParameterKeys[],
                               const char* const queryParamValues[],
                               uint16_t port  = HTTP_DEFAULT_PORT);

    /** read data from HTTP GET response
        1 AT+HTTPREAD
        @param buffer: buffer where the data will be copied to (zero-terminated)
        @param bufferSize: size of the passed buffer

        @returns true if successful, false if there was a timeout or the passed buffer was too small
    */
    bool httpReadResponseData(char* buffer, uint16_t bufferSize);

    //////////////////////////////////////////////////////
    /// others
    //////////////////////////////////////////////////////
    void AT_Bypass();
  private:
    /** queries bearer: sends AT+SAPBR=2,1 and checks returned data, sets member variables _ip and ip_string
        if we are connected
        @param bearerStatus: output parameter, status of the bearer is written to this pointer:
         0 bearer is connecting
         1 bearer is connected
         2 bearer is closing
         3 bearer is closed
        @returns true if successful, false if something was wrong

        NOTE: maybe made public if desperately required (should not be required however)
    */
    bool queryBearer(uint8_t* bearerStatus);

    uint32_t str_to_ip(const char* str);

    SoftwareSerial gprsSerial;
    static GPRS* inst;
    uint32_t _ip;
    char ip_string[16]; //XXX.YYY.ZZZ.WWW + \0
};
#endif
