/*******************************************************************************************
  Device.ino
  notions:
    - Changes required by developer
      - "DEVICE_SERIAL_NO" (device serial number)
      - "PUBLISH_INTERVAL" ( data uploading interval)

Files:
   IO.ino :  configure IO, pub/sub for data
   SocketIO.ino : no needs modifications.

Versio : 1.0
Updats : 1.1  cshanges socketIO from http to https
Changes :  Input/Output port mapping is re-defined
           - output commands from front-end app view:
                 value : 1  as active
                 value : 0  as inactive
           - device activation is defined with  "IO_ACTIVE", default "low active"
***********************************************************************************/
#include "defs.h"
#define PUBLISH_INTERVAL 100000            // 10 seconds, if 0 ,no timed  publishing
#define WIFI_SSID "DAMOSYS"                // wifi AP SSID
#define WIFI_PWD "damo8864"                // wifi AP password
#define DEVICE_SERIAL_NO "sn-esp8266-io11" // change with your device serival #

#define MAX_INPUT 1  // how many inputs
#define MAX_OUTPUT 1 // hot many outputs
#define MAX_IO (MAX_INPUT + MAX_OUTPUT)

#define INPUT_0 4   // gpio 4 as input
#define OUTPUT_0 13 // gpio 13 as output

#define IO_ACTIVE 0                           // set IO low active or 1 as active high ,
#define ACTIVE_STATUS(x) (IO_ACTIVE ? !x : x) // frontend app, 1: on, 0: off

typedef struct _tag
{
  uint8_t pin;
  uint8_t type;
  uint8_t state;
} PortMap;
// begins with input first
PortMap _portMap[] = {{INPUT_0, INPUT_PULLUP, 0}, {OUTPUT_0, OUTPUT, 0}};

uint32_t _tsLastPublished = 0;
bool _bSocketConnected = false;
bool _bDataPublishRequired = false;
bool _bStatusPublishRequired = false;

//-------------------------------------------------------------------------
// name  : connectionCallback
void connectionCallback(bool status)
{
  debugF("%s: %s\n", (char *)__FUNCTION__, status ? "connected" : "dis-connected");
  if (status)
    _bStatusPublishRequired = status;
  _bSocketConnected = status;
}
//-------------------------------------------------------------------------
// name  : eventCallback
void eventCallback(String jsonStr)
{
  const int capacity = JSON_ARRAY_SIZE(3) + 4 * JSON_OBJECT_SIZE(4) + 10 * JSON_OBJECT_SIZE(1);
  StaticJsonDocument<capacity> doc;
  DeserializationError err = deserializeJson(doc, jsonStr);
  if (err)
  {
    debugF("%s: %s\n", "deserializeJson() returned ", (const char *)err.f_str());
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
  else if (strcmp(cmd, "chat") == 0)
  {
  }
}
//-------------------------------------------------------------------------
// name  : initIO
void initIO()
{
  for (int i = 0; i < MAX_IO; i++)
  {
    pinMode(_portMap[i].pin, _portMap[i].type);
    if (_portMap[i].type == OUTPUT)
      setOutput(i, _portMap[i].state);
    else
      _portMap[i].state = digitalRead(_portMap[i].pin); // just read signal only
  }
}
//-------------------------------------------------------------------------
// name  : setOutput
void setOutput(uint8_t portIdx, uint8_t state)
{
  if (portIdx >= 0 && portIdx < MAX_IO)
  {
    if (_portMap[portIdx].type == OUTPUT)
    {
      _portMap[portIdx].state = state ? 1 : 0;
      debugF("portIndex=%d  portState=%d", portIdx, _portMap[portIdx].state);
      digitalWrite(_portMap[portIdx].pin, ACTIVE_STATUS(_portMap[portIdx].state));
      _bDataPublishRequired = true;
    }
#ifdef DEBUG
    else
      debugF("%s: %s\n", (char *)__FUNCTION__, "invalid cmd, input is not applicable");
#endif
  }
#ifdef DEBUG
  else
    debugF("%s: %s\n", (char *)__FUNCTION__, (char *)__FUNCTION__, "invalid portIdx");
#endif
}
//-------------------------------------------------------------------------
// name  : setOutputAll
void setOutputAll(uint8_t state)
{
  for (int i = MAX_INPUT; i < MAX_IO; i++)
    setOutput(i, state);
}
//-------------------------------------------------------------------------
// name  : checkInputUpdated
bool checkInputUpdated()
{
  uint8_t value;
  bool updated = false;
  for (int i = 0; i < MAX_INPUT; i++)
  {
    value = digitalRead(_portMap[i].pin);
    if (value != _portMap[i].state)
    {
      _portMap[i].state = value;
      updated = true;
    }
  }
  return updated;
}
//-------------------------------------------------------------------------
// name  : publishData
void publishData(uint32_t now)
{
  char szBuf[128];
  sprintf(szBuf, "[%d,%d]", ACTIVE_STATUS(_portMap[0].state), _portMap[1].state);
  publish(DATA_EVENT, szBuf);
  _bDataPublishRequired = false;
  _tsLastPublished = now;
}
//-------------------------------------------------------------------------
// name  : setup
void setup()
{
  Serial.begin(115200);
  debugF("working with %s\n", IO_ACTIVE ? "ACTIVE_HIGH" : "ACTIVE_LOW");
  initIO();

  if (WiFi.getMode() & WIFI_AP)
    WiFi.softAPdisconnect(true);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PWD);

  while (WiFiMulti.run() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  initSocketIO(DEVICE_SERIAL_NO, &eventCallback, &connectionCallback);
}
//-------------------------------------------------------------------------
// name  : loop
void loop()
{
  uint32_t now = millis();

  if (_bDataPublishRequired || ((now - _tsLastPublished) > 10000) || checkInputUpdated())
    publishData(now);
  if (_bStatusPublishRequired)
  {
    publish(STATUS_EVENT, "Connected");
    _bStatusPublishRequired = false;
  }
  socketIOLoop();
}