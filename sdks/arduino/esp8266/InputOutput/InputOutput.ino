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
   InputOutput.ino :  it can be modified for user's application
   SocketIO.ino : no modifications.
   defs.h :  _IO_CONTROL_, DEBUG can be modified.
*****************************************************************************************************************************/
#include "defs.h"

#define PUBLISH_INTERVAL 100000 // 10 seconds, if 0 then no "timed data publishing"

#define WIFI_SSID "DAMOSYS"
#define WIFI_PWD "damo8864"

#ifdef _IO_CONTROL_

#ifdef _BULB_CONTROL_
#define SERIAL_NO "sn-esp8266-light" // change with your device serival #
#else
#define SERIAL_NO "sn-esp8266-io" // change with your device serival #
#endif
// GPIO pin definitions ---------------------------------------------
#define OUTPUT_0 14
#define OUTPUT_1 13
#define OUTPUT_2 16
#define INPUT_0 5
#define INPUT_1 4
#define INPUT_2 3

#define MAX_INPUT 3
#define MAX_OUTPUT 3
#define MAX_IO (MAX_INPUT + MAX_OUTPUT)

typedef struct _tag
{
  uint8_t pin;
  uint8_t type;
  uint8_t state;
} Port;

// setup Pin map
Port _ports[MAX_IO] = {
    {INPUT_0, INPUT_PULLUP, 1},
    {INPUT_1, INPUT_PULLUP, 1},
    {INPUT_2, INPUT_PULLUP, 1},
    {OUTPUT_0, OUTPUT, 0},
    {OUTPUT_1, OUTPUT, 0},
    {OUTPUT_2, OUTPUT, 0},
};
#else
#define SERIAL_NO "sn-esp8266-sensors" // change with your device serival #
float _curTempreture = 0;
float _curHumidity = 0;
float readSensor(int min, int max, float divider)
{
  return random(min, max) / divider; // make fake data for test
}
#endif

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
#ifdef _IO_CONTROL_
  for (int i = 0; i < MAX_IO; i++)
  {
    pinMode(_ports[i].pin, _ports[i].type);
    if (_ports[i].type == OUTPUT)
      setOutput(i, HIGH);
    else
      _ports[i].state = digitalRead(_ports[i].pin);
  }
#ifdef _BULB_CONTROL_
  // attachInterrupt(digitalPinToInterrupt(INPUT_MOTION), isr_motion, FALLING);
  attachInterrupt(digitalPinToInterrupt(INPUT_0), isr_0, FALLING);
  attachInterrupt(digitalPinToInterrupt(INPUT_1), isr_1, FALLING);
  attachInterrupt(digitalPinToInterrupt(INPUT_2), isr_2, FALLING);
#endif
#endif

  initSocketIO(SERIAL_NO, PUBLISH_INTERVAL);
}
#ifdef _IO_CONTROL_
#ifdef _BULB_CONTROL_
void isrProc(int inputIndex)
{
  // set input flag as low,  interrupted with FALLING-EDGE
  //_ports[inputIndex].state = ! _ports[inputIndex].state;
  _ports[inputIndex].state = !_ports[inputIndex].state;

  // set output toggled by input
  int outputIdx = MAX_INPUT + inputIndex;
  setOutput(outputIdx, !_ports[outputIdx].state);
  setPublish(true);
}
ICACHE_RAM_ATTR void isr_0() { isrProc(0); }
ICACHE_RAM_ATTR void isr_1() { isrProc(1); }
ICACHE_RAM_ATTR void isr_2() { isrProc(2); }
#else
//----------------------------------------------------------------------
void ioLoop()
{
  int v;
  bool bStateChanged = false;
  for (int i = 0; i < MAX_INPUT; i++)
  {
    v = digitalRead(_ports[i].pin);
    if (_ports[i].state != v)
    {
      _ports[i].state = v;
      bStateChanged = true;
    }
  }
  if (bStateChanged)
    setPublish(true);
}
#endif
#endif

//----------------------------------------------------------------------
void loop()
{
#if (defined _IO_CONTROL_ && ! defined _BULB_CONTROL_)
  ioLoop(); // input signal polling,
#endif
  socketIOLoop(); // work for networking communication
}

/***********************************************************************
 fuations: called by socketIO.ino
***********************************************************************/
//-----------------------------------------------------------------------
// desc : called "timed data publishing" by "PUBLISH_INTERVAL"
//        or setPublish(true) called by user call( manual call)
//        fill your IO data to szBuf
void getDeviceData(char *szBuf)
{
#ifdef _IO_CONTROL_
  sprintf(szBuf, "[%d,%d,%d,%d,%d,%d]",
          _ports[0].state, _ports[1].state, _ports[2].state,
          _ports[3].state, _ports[4].state, _ports[5].state);
#else
  _curTempreture = readSensor(-20, 1000, 10);
  _curHumidity = readSensor(-20, 1000, 10);
  sprintf(szBuf, "[%3.2f,%3.2f]", _curTempreture, _curHumidity);
#endif
}

//-----------------------------------------------------------------------
// desc : reboot
void reboot()
{
  ESP.restart(); // ESP.reset();
}

#ifdef _IO_CONTROL_
//-----------------------------------------------------------------------
// field : output port index, begins with 0
void setOutput(uint8_t field, uint8_t state)
{
  if (field < 0 || field >= MAX_IO)
  {
    debug_out2((char *)__FUNCTION__, "invalid field");
    return;
  }

  if (_ports[field].type == OUTPUT)
  {
    _ports[field].state = state;
    digitalWrite(_ports[field].pin, state);
  }
  else
  {
    debug_out2(__FUNCTION__, "port type invalid");
  }
}

//-----------------------------------------------------------------------
// desc : set all output port with 'value'
// value : 0 or 1
void setOutputAll(uint8_t state)
{
  for (int i = MAX_INPUT; i < MAX_IO; i++)
    setOutput(i, state);
}
#endif