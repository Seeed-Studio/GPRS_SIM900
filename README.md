##GPRS Shield and Cheap SIM800L PCB BOARDS

Only two special functions are different for SIM900 Shield or SIM800L PCB boards:
  - for SIM900 shield you have to use powerUpDown
  - for SIM800L with RST pin, you have to use powerReset

[![Shield - GPRS Shield V2.0](http://www.seeedstudio.com/depot/images/product/GRPS%20SLD_02.jpg)](http://goo.gl/UC8DkO)
GPRS Shield provide you a way to DIY your own **mobile phone**. You can use your GRPS Shield to dial a phone number or send a text message to your friend. Besides these, you can use it to access the network and view web pages. For more information, you can visit our [Wiki Page](http://goo.gl/U2I1d2)
### Features
+ Compatible with standard Arduino and Arduino Mega
+ Selectable interface between hardware serial port and software serial port
+ Selectable interface between hardware serial port and software serial port
+ Support TCP/UDP protocols
+ Full control via AT commands set: Standard - GSM 07.07 & 07.05 and Enhanced - SIMCOM AT Commands
+ 2-in-1 headset jack
+ Convenient external SIM card holder
+ Low power consumption: 1.5mA(sleep mode)
+ 3V CR1220 Battery Holder Connetor

### Applications
+ Call up and answer Call
+ Send SMS and read SMS
+ AT Command Test
+ TCP Connection Test 

### Getting Started
Make sure that two libraries ([GPRS_Shield](https://github.com/Seeed-Studio/GPRS_Shield) —— GPRS Module Library, [SoftwareSerial](https://github.com/arduino/Arduino/tree/master/libraries/SoftwareSerial) —— we use software serial to communicate with GPRS module) have been in your libraries folder. Then you can have a test with the sketches in example folder.
+ GPRS_Serial_Debug —— test GPRS module with AT commands.
+ GPRS_CallUp ——  give a call to someone.
+ GPRS_SendSMS —— send a message to someone.
+ GPRS_LoopHandle —— answer the call & read the message.
+ GPRS_SMSread —— another way to read a message. 
+ GPRS_TCPConnection —— build a TCP connection and read the content from the specified website. 


----

This software is written by lawliet zou (![](http://www.seeedstudio.com/wiki/images/f/f8/Email-lawliet.zou.jpg)) for [Seeed Technology Inc.](http://www.seeed.cc) and is licensed under [The MIT License](http://opensource.org/licenses/mit-license.php). Check License.txt/LICENSE for the details of MIT license.<br>

Contributing to this software is warmly welcomed. You can do this basically by<br>
[forking](https://help.github.com/articles/fork-a-repo), committing modifications and then [pulling requests](https://help.github.com/articles/using-pull-requests) (follow the links above<br>
for operating guide). Adding change log and your contact into file header is encouraged.<br>
Thanks for your contribution.

Seeed is a hardware innovation platform for makers to grow inspirations into differentiating products. By working closely with technology providers of all scale, Seeed provides accessible technologies with quality, speed and supply chain knowledge. When prototypes are ready to iterate, Seeed helps productize 1 to 1,000 pcs using in-house engineering, supply chain management and agile manufacture forces. Seeed also team up with incubators, Chinese tech ecosystem, investors and distribution channels to portal Maker startups beyond.



[![Analytics](https://ga-beacon.appspot.com/UA-46589105-3/GPRS_Shield)](https://github.com/igrigorik/ga-beacon)

