/*
 * This examples shows how to publish data and status
 */

#include <AtCloudIO.h>
#define DEBUG

#define SERIAL_NO "sn-esp8266-light-with-lib" // change with your device serival #

#define INPUT_0 4
#define OUTPUT_0 13

#define MAX_INPUT 1
#define MAX_OUTPUT 1
#define MAX_IO (MAX_INPUT + MAX_OUTPUT)
#define OUTPUT_BEGIN MAX_INPUT

#define ACTIVE_LOW 0
#define SIGINAL_ACTIVE ACTIVE_LOW
#define ACTIVE_STATUS(x) (SIGINAL_ACTIVE == ACTIVE_LOW ? !x : x)

typedef struct _tag
{
  uint8_t pin;
  uint8_t type;
  uint8_t state;
} PortMap;
PortMap _portMap[] = {{INPUT_0, INPUT_PULLUP, 0}, {OUTPUT_0, OUTPUT, 0}};

uint32_t _dPublishInterval = 10000;
uint32_t _tsLastPublished = 0;

bool _bDataPublishRequired = false;
bool _bStatusPublishRequired = false;
bool _bSocketConnected = false;

int _inputValueUserView;
int _outputValueUserView = 0;

AtCloudIO _atCloudIO(SERIAL_NO);

void onEvent(const socketIOmessageType_t &type, uint8_t *payload, const size_t &length)
{
    switch (type)
    {
    case sIOtype_DISCONNECT:
        //debug_out2(("[IOc]", "Disconnected");
        _bSocketConnected = false;
        //(*cbSocketConnection)(0);
        break;
    case sIOtype_CONNECT:
        //debug_out2(("[IOc] Connected to url: ", (const char *)payload);

        if (_bSocketConnected)
        {
            _bSocketConnected = false;
            // pubStatus("System Ready");
        }
        //(*cbSocketConnection)(1);
        _bSocketConnected = true;
        break;
    case sIOtype_EVENT:
        //debug_out2(("[IOc] Got event: ", (char *)payload);
        {
            // payload like : ["app-cmd",{"cmd":"sync","content":""}]
            if (payload[2] != 'a')
                return; // only for app-cmd
            String text = ((const char *)&payload[0]);
            // processEvent(text.substring(text.indexOf('{'), text.length() - 1));
            // if (cbSocketDataReceived)
            //     (*cbSocketDataReceived)(text.substring(text.indexOf('{'), text.length() - 1));
        }
        break;
    case sIOtype_ACK:
        //debugHexDump("[IOc] Get ack: ", payload, length);
        break;
    case sIOtype_ERROR:
        //debugHexDump("[IOc] Get error: ", payload, length);
        break;
    case sIOtype_BINARY_EVENT:
        //debugHexDump("[IOc] Get binary: ", payload, length);
        break;
    case sIOtype_BINARY_ACK:
        //debugHexDump("[IOc] Get binary ack: ", payload, length);
        break;
    case sIOtype_PING:
        //debug_out2(("[IOc]", "Got PING");
        break;
    case sIOtype_PONG:
        //debug_out2(("[IOc]", "Got PONG");
        break;
    default:
        break;
    }
}
void connectionCallback(bool status)
{
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
    ////debug_out2(("deserializeJson() returned ", (const char *)err.f_str());
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
    // stopSocketIO();
    _atCloudIO.disConnect();
    ESP.restart();
  }
  else if (strcmp(cmd, "chat") == 0)
  {
  }
}
void setup()
{
  for (int i = 0; i < MAX_IO; i++)
  {
    pinMode(_portMap[i].pin, _portMap[i].type);
    if (_portMap[i].type == OUTPUT)
      setOutput(i, _portMap[i].state);
    else
      _portMap[i].state = ACTIVE_STATUS(digitalRead(_portMap[i].pin)); // just read signal only
  }
  _atCloudIO.onEvent(onEvent);
  //_atCloudIO.init(&eventCallback, &connectionCallback);
  _atCloudIO.init();
}

void setOutput(uint8_t portIdx, uint8_t state)
{
  if (portIdx >= 0 && portIdx < MAX_IO)
  {
    if (_portMap[portIdx].type == OUTPUT)
    {
      _portMap[portIdx].state = state ? 1 : 0;
      // debugF("portIndex=%d  portState=%d", portIdx, _portMap[portIdx].state);
      digitalWrite(_portMap[portIdx].pin, ACTIVE_STATUS(_portMap[portIdx].state));
      _bDataPublishRequired = true;
    }
    else
    {
      ////debug_out2(((char *)__FUNCTION__, "invalid cmd, port is not mapped as output");
    }
  }
#ifdef DEBUG
  //else
    ////debug_out2(((char *)__FUNCTION__, "invalid portIdx");
#endif
}
void setOutputAll(uint8_t state)
{
  for (int i = OUTPUT_BEGIN; i < MAX_IO; i++)
    setOutput(i, state);
}
//----------------------------------------------------------------
// in App client widget  display
// _inputValueUserView, _inputValueUserView  =>   0="OFF",  1="ON"
//----------------------------------------------------------------
void loop()
{

  uint32_t now = millis();
  if ((now - _tsLastPublished) > _dPublishInterval)
  {
    char szBuf[128];
    sprintf(szBuf, "[%d,%d]", ACTIVE_STATUS(digitalRead(_portMap[0].pin)), _portMap[1].pin);
    _atCloudIO.publishData(szBuf);
    _tsLastPublished = now;
  }
  if (_bStatusPublishRequired)
  {
    _atCloudIO.publishStatus("Connected");
    _bStatusPublishRequired = false;
  }
  // socketIOLoop();
  _atCloudIO.loop();
}
