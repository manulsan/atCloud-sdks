#include "defs.h"

#define DATA_EVENT "dev-data"
#define STATUS_EVENT "status-data"

#define _WEBSOCKETS_LOGLEVEL_ 0 // 0-4
#define SIO_PATH "/api/dev/io/" // do not change

bool _bSocketConnected = false;
bool _bInitialConnection = true;
bool _bPublishRequired = false;
uint32_t _dPublishingInterval = 0;
uint32_t _tsLastPublishing = 0;

void initSocketIO(char *szDeviceNo, uint32_t dInterval)
{
    _ntpClient.begin(); // begin NTP client for time sync
    _socketIO.onEvent(onEvent);
    _socketIO.setReconnectInterval(10000);

    _dPublishingInterval = dInterval;

    char szBuf[128];
    sprintf(szBuf, "%s?sn=%s", SIO_PATH, szDeviceNo);
    _socketIO.begin(SERVER_URL, SERVER_PORT, szBuf);
}

/*----------------------------------------------------------------------
cmd from user via platform
example  : ["app-cmd",{"cmd":"output","content":{"field":3,"value":0}}]
----------------------------------------------------------------------*/
void processEvent(String jsonStr)
{
    const int capacity = JSON_ARRAY_SIZE(2) + 2 * JSON_OBJECT_SIZE(3) + 4 * JSON_OBJECT_SIZE(1);
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
#ifdef _IO_CONTROL_
        auto f = doc["content"]["field"].as<int>();
        auto v = doc["content"]["value"].as<int>();
        setOutput(f, v);
        setPublish(true);
#endif
    }
    else if (strcmp(cmd, "output-all") == 0)
    {
#ifdef _IO_CONTROL_
        auto v = doc["content"]["value"].as<int>();
        setOutputAll(v);
        setPublish(true);
#endif
    }
    else if (strcmp(cmd, "reboot") == 0)
    {
        _socketIO.send(sIOtype_DISCONNECT, "");
        reboot();
    }
    else if (strcmp(cmd, "sync") == 0)
        setPublish(true);
    else if (strcmp(cmd, "chat") == 0)
        debug_out2("do chat", "");
}
//-------------------------------------------------------------
void onEvent(const socketIOmessageType_t &type, uint8_t *payload, const size_t &length)
{
    switch (type)
    {
    case sIOtype_DISCONNECT:
        debug_out2("[IOc]", "Disconnected");
        _bSocketConnected = false;
        break;
    case sIOtype_CONNECT:
        debug_out2("[IOc] Connected to url: ", (const char *)payload);

        if (_bInitialConnection)
        {
            _bInitialConnection = false;
            // pubStatus("System Ready");
        }
        _bSocketConnected = true;
        break;
    case sIOtype_EVENT:
        debug_out2("[IOc] Got event: ", (char *)payload);
        {
            // payload like : ["app-cmd",{"cmd":"sync","content":""}]
            String text = ((const char *)&payload[0]);
            processEvent(text.substring(text.indexOf('{'), text.length() - 1));
        }
        break;
    case sIOtype_ACK:
        debugHexDump("[IOc] Get ack: ", payload, length);
        break;
    case sIOtype_ERROR:
        debugHexDump("[IOc] Get error: ", payload, length);
        break;
    case sIOtype_BINARY_EVENT:
        debugHexDump("[IOc] Get binary: ", payload, length);
        break;
    case sIOtype_BINARY_ACK:
        debugHexDump("[IOc] Get binary ack: ", payload, length);
        break;
    case sIOtype_PING:
        debug_out2("[IOc]", "Got PING");
        break;
    case sIOtype_PONG:
        debug_out2("[IOc]", "Got PONG");
        break;
    default:
        break;
    }
}
void publish(char *szEventName, char *szContent)
{
    _bPublishRequired = false;

    DynamicJsonDocument doc(1024);
    JsonArray root = doc.to<JsonArray>();
    root.add(szEventName);

    JsonObject jsonObj = root.createNestedObject();

    if (szContent[0] == '[') // data begins with '[' regarding as array and should be serialized
        jsonObj["content"] = serialized(szContent);
    else
        jsonObj["content"] = szContent;

    _ntpClient.update();                                               // update time value
    jsonObj["createdAt"] = (uint64_t)_ntpClient.getEpochTime() * 1000; // set seconds to millisecond value, require 64bit

    String output;
    serializeJson(doc, output); // JSON to String (serializion)
    _socketIO.sendEVENT(output);

    debug_out2("Pub: ", output.c_str());
}

//-------------------------------------------------------------
// ex> szData="[0,0,1,1]" , array expression
void pubData(char *szData)
{
    publish(DATA_EVENT, szData);
}

//-------------------------------------------------------------
void pubStatus(char *szData)
{
    publish(STATUS_EVENT, szData);
}

//-------------------------------------------------------------
void setPublish(bool bFlag)
{
    _bPublishRequired = bFlag;
}
//-------------------------------------------------------------
void setTimedPublish(uint32_t now)
{
    if (_dPublishingInterval && ((now - _tsLastPublishing) > _dPublishingInterval))
    {
        _bPublishRequired = true;
        _tsLastPublishing = now;
    }
}
//-------------------------------------------------------------
void socketIOLoop()
{
    _socketIO.loop();
    setTimedPublish(millis());

    if (_bPublishRequired)
    {
        char szBuf[256] = {};
        getDeviceData(szBuf);
        pubData(szBuf);
    }
}