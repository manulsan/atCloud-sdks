/*******************************************************************************************
  Device.ino
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

Versio : 1.0
Updats : 1.1 / socketIO over https
Changes :  Input/Output port map is merged again
***********************************************************************************/
#include "defs.h"
#define PUBLISH_INTERVAL 100000 // 10 seconds, if 0 ,no timed  publishing
#define WIFI_SSID "DAMOSYS"
#define WIFI_PWD "damo8864"
//#define SERIAL_NO "your-device-device-serial-no" // change with your device serival #, (sn-esp8266-io)
#define SERIAL_NO "sn-esp8266-io" // change with your device serival #

#define INPUT_0 5
#define INPUT_1 4
#define INPUT_2 3
#define OUTPUT_0 14
#define OUTPUT_1 13
#define OUTPUT_2 16
#define OUT_DEFAULT 1
#define MAX_INPUT 3
#define MAX_OUTPUT 3
#define OUTPUT_BEGIN MAX_INPUT

#define MAX_IO (MAX_INPUT + MAX_OUTPUT)
Port _portMap[] = {{INPUT_0, INPUT_PULLUP, 0}, {INPUT_1, INPUT_PULLUP, 0}, {INPUT_2, INPUT_PULLUP, 0}, {OUTPUT_0, OUTPUT, OUT_DEFAULT}, {OUTPUT_1, OUTPUT, OUT_DEFAULT}, {OUTPUT_2, OUTPUT, OUT_DEFAULT}};
bool _bSocketConnected = false;
bool _bDataPublishRequired = false;
bool _bStatusPublishRequired = false;

uint32_t _tsLastPublished = 0;
uint32_t _dPublishInterval = 10000;
extern uint8_t publish(uint8_t eventType, char *szContent);

void isrProc(int portIdx)
{
  _portMap[portIdx].state = !_portMap[portIdx].state;
  _bDataPublishRequired = true;
}

ICACHE_RAM_ATTR void isr_0() { isrProc(0); }
ICACHE_RAM_ATTR void isr_1() { isrProc(1); }
ICACHE_RAM_ATTR void isr_2() { isrProc(2); }

void connectionCallback(bool status)
{
  debug_out2(__FUNCTION__, status ? "connected" : "dis-connected");
  if (status)
    _bStatusPublishRequired = status;
  _bSocketConnected = status;
}

void eventCallback(String jsonStr)
{
  const int capacity = JSON_ARRAY_SIZE(3) + 4 * JSON_OBJECT_SIZE(4) + 10 * JSON_OBJECT_SIZE(1);
  StaticJsonDocument<capacity> doc;
  DeserializationError err = deserializeJson(doc, jsonStr);
  if (err)
  {
    debug_out2("deserializeJson() returned ", (const char *)err.f_str());
    return;
  }
  auto cmd = doc["cmd"].as<const char *>();
  if (strcmp(cmd, "output") == 0)
  {
    auto f = doc["content"]["field"].as<int>();
    auto v = doc["content"]["value"].as<int>();
    setOutput(f, v);
  }
  else if (strcmp(cmd, "output-all") == 0)
  {
    auto v = doc["content"]["value"].as<int>();
    setOutputAll(v);
  }
  else if (strcmp(cmd, "sync") == 0)
    _bDataPublishRequired = true;
  else if (strcmp(cmd, "reboot") == 0)
  {
    stopSocketIO();
    ESP.restart();
  }
  // else if (strcmp(cmd, "chat") == 0)  {}
}

void setOutput(uint8_t portIdx, uint8_t state)
{
  if (portIdx >= 0 && portIdx < MAX_IO)
  {
    if (_portMap[portIdx].type == OUTPUT)
    {
      _portMap[portIdx].state = state ? 1 : 0;
      digitalWrite(_portMap[portIdx].pin, _portMap[portIdx].state);
      _bDataPublishRequired = true;
    }
    else
    {
      debug_out2((char *)__FUNCTION__, "invalid cmd, port is not mapped as output");
    }
  }
  else
  {
    debug_out2((char *)__FUNCTION__, "invalid portIdx");
  }
}

void setOutputAll(uint8_t state)
{
  for (int i = OUTPUT_BEGIN; i < MAX_IO; i++)
    setOutput(i, state);
}

void publishData(uint32_t now)
{
  char szBuf[128];
  sprintf(szBuf, "[%d,%d,%d,%d,%d,%d]",
          _portMap[0].state, _portMap[1].state, _portMap[2].state,
          _portMap[3].state, _portMap[4].state, _portMap[5].state);

  if (_bSocketConnected)
    publish(DATA_EVENT, szBuf);
  else
  {
    debug_out2(__FUNCTION__, "not connected status");
  }
  _bDataPublishRequired = false;
  _tsLastPublished = now;
}

void publishStatus(char *szBuf)
{
  publish(STATUS_EVENT, szBuf);
  _bStatusPublishRequired = false;
}

void setup()
{
  Serial.begin(115200);
  
  initIO();

  if (WiFi.getMode() & WIFI_AP)
    WiFi.softAPdisconnect(true);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PWD);

  while (WiFiMulti.run() != WL_CONNECTED)
  {
    debug_out1(".");
    delay(500);
  }
}

void initIO()
{
  int ledStatus = 0;
  for (int i = 0; i < MAX_IO; i++)
  {
    pinMode(_portMap[i].pin, _portMap[i].type);
    if (_portMap[i].type == OUTPUT)
      digitalWrite(_portMap[i].pin, _portMap[i].state);
    else
      _portMap[i].state = digitalRead(_portMap[i].pin);
  }
  attachInterrupt(digitalPinToInterrupt(INPUT_0), isr_0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(INPUT_1), isr_1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(INPUT_2), isr_2, CHANGE);

  initSocketIO(SERIAL_NO, &eventCallback, &connectionCallback);
}

void loop()
{
  uint32_t now = millis();
  if (_bDataPublishRequired || ((now - _tsLastPublished) > _dPublishInterval))
    publishData(now);
  if (_bStatusPublishRequired)
    publishStatus("Connected");
  socketIOLoop();
}