/*******************************************************************************************
  inputoutput.ino
  Device : ESP32 ( Olimex ESP32 EVB )
  Desc : 1 input and 2 output(relay) controls.
  notions:
    - "SERIAL_NO" requires device serial number
    - "PUB_INTERVAL_MILLIS"
         if values is 0 then no no automaticcal publishing
         otherwise data is published every interval
    - getDeviceData is called every interval
    - if manual publishin required, call setPublish(true)

Files:
   InputOutput.ino :  it can be modified for user's application
   SocketIO.ino : no modifications required.
   defs.h :  header file.
Versio : 1.0
Updats : 1.1 / socketIO over https
Changes :  Input/Output port map is merged again
***********************************************************************************/

#include "defs.h"
#define WIFI_SSID "DAMOSYS"
#define WIFI_PWD "damo8864"
// #define SERIAL_NO "your-device-device-serial-no" // change with your device serival #, (sn-esp8266-io)
#define SERIAL_NO "sn-io-esP32-device" // change with your device serival #

#define OUTPUT_0 32
#define OUTPUT_1 33
#define INPUT_0 34

#define MAX_INPUT 1
#define MAX_OUTPUT 2
#define OUTPUT_BEGIN MAX_INPUT

//-- !import, change function publishData with MAX_IO
#define MAX_IO (MAX_INPUT + MAX_OUTPUT)
//--------------------------------------------------------------

Port _portMap[] = {{INPUT_0, INPUT_PULLUP, 0}, {OUTPUT_0, OUTPUT, 0}, {OUTPUT_1, OUTPUT, 0}};

#define PUB_INTERVAL_MILLIS 100000              // milli-seconds
uint32_t _tsLastPublished = 0;
extern uint8_t publish(uint8_t eventType, char *szContent);

bool _bSocketConnected = false;
bool _bDataPublishRequired = false;
bool _bStatusPublishRequired = false;

void isrProc(int portIdx)
{
  // debug_out2("isrProc","called");
  _portMap[portIdx].state = !_portMap[portIdx].state;
  _bDataPublishRequired = true;
}

ICACHE_RAM_ATTR void isr_0() { isrProc(0); }

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
  // int and float, both are  for expression of json string
#if 1
  //  if user as as input/output, it shoule is bes as integer
  sprintf(szBuf, "[%d,%d,%d]", _portMap[0].state, _portMap[1].state, _portMap[2].state);
#else //  if user as "gathering device" with values with float , data is array of float
  sprintf(szBuf, "[%.2f,%.2f,%.2f]", (float)_portMap[0].state, (float)_portMap[1].state, (float)_portMap[2].state);
#endif
  if (_bSocketConnected)
    publish(DATA_EVENT, szBuf);
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
  Serial.print("Setup is called");

  WiFi.begin(WIFI_SSID, WIFI_PWD);

  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nWiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  initIO();
  initSocketIO(SERIAL_NO, &eventCallback, &connectionCallback);
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
  // attachInterrupt(digitalPinToInterrupt(INPUT_0), isr_0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(INPUT_0), isr_0, FALLING); // LOW, CHANGE, RISING, FALLING, ,HIGH
  // attachInterrupt(digitalPinToInterrupt(INPUT_0), isr_0, RISING); // LOW, CHANGE, RISING, FALLING, ,HIGH
}

void loop()
{
  uint32_t now = millis();
  if (_bDataPublishRequired || ((now - _tsLastPublished) > PUB_INTERVAL_MILLIS))
    publishData(now);
  if (_bStatusPublishRequired)
    publishStatus("Connected");
  socketIOLoop();
}