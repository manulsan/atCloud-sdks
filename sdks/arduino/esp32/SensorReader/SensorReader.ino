/****************************************************************************************************************************
  Device.ino
  Based on and modified from WebSockets libarary https://github.com/Links2004/arduinoWebSockets
  to support other boards such as  SAMD21, SAMD51, Adafruit's nRF52 boards, etc.

  Built by Khoi Hoang https://github.com/khoih-prog/WebSockets_Generic
  Licensed under MIT license

  Originally Created on: 06.06.2016
  Original Author: Markus Sattler

  User's device working code.
  notions:
    - "SERIAL_NO" requires device serial number
    - "PUBLISH_INTERVAL"
         if values is 0 then no no automaticcal publishing
         otherwise data is published every interval
    - getDeviceData is called every interval
    - if manual publishin required, call setPublish(true)

Files:
   SensorReader.ino: it can be modified for user's application
   SocketIO.ino : no need modifications.
   defs.h :  _GPIO_USED_, DEBUG can be modified.    
*****************************************************************************************************************************/
#include "defs.h"
#define SERIAL_NO "sn-esp8266-sensors" // change with your device serival #
#define PUBLISH_INTERVAL 10000    // 10 seconds, if 0 then no "timed data publishing"

#define WIFI_SSID "DAMOSYS"
#define WIFI_PWD "damo8864"

float _curTempreture = 0;
float _curHumidity = 0;

//----------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);  
  debug_out2(ARDUINO_BOARD, WEBSOCKETS_GENERIC_VERSION);

  if (WiFi.getMode() & WIFI_AP)
    WiFi.softAPdisconnect(true);

  WiFiMulti.addAP(WIFI_SSID, WIFI_PWD);
  while (WiFiMulti.run() != WL_CONNECTED)
  {
    debug_out1(".");
    delay(100);
  }
  initDevice();
}

//----------------------------------------------------------------------
void initDevice()
{
  initSocketIO(SERIAL_NO, PUBLISH_INTERVAL);
}
//----------------------------------------------------------------------
// this is fake data generator, make your own code with your sensor
float readSensor(int min, int max, float divider)
{
  return random(min, max) / divider; // make fake data for test
}

//----------------------------------------------------------------------
void loop()
{
  if (WiFiMulti.run() == WL_CONNECTED) // deviceLoop();
    socketIOLoop();
}

/***********************************************************************
 fuations: called by socketIO.ino
           ( for GPIO controL, refer sample project "InputOutput")
***********************************************************************/
//-----------------------------------------------------------------------
// desc : call at "timed data publishing" or setPublish(true) called
// expression "%3.2f" is sample, any of numeric(integerm float) is acceptable.
void getDeviceData(char *szBuf)
{
  _curTempreture = readSensor(-20, 1000, 10);
  _curHumidity = readSensor(-20, 1000, 10);
  sprintf(szBuf, "[%3.2f,%3.2f]", _curTempreture, _curHumidity);
}

//-----------------------------------------------------------------------
// desc : reboot
void reboot()
{
  ESP.restart(); // ESP.reset();
}

#ifdef _GPIO_USED_
//-----------------------------------------------------------------------
// field : output port index, begins with 0
void setOutput(uint8_t field, uint8_t value)
{
  // refer InputOutput example project
}

//-----------------------------------------------------------------------
// desc : set all output port with 'value'
// value : 0 or 1
void setOutputAll(uint8_t value)
{
  // refer InputOutput example project
}
#endif
