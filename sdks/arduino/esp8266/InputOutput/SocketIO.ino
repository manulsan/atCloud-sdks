#include "defs.h"
#define _WEBSOCKETS_LOGLEVEL_ 0   // 0-4
#define _SIO_PATH_ "/api/dev/io/" // do not change

bool _bSocketIOConnected = false;
bool _bInitialConnection = true;
static void (*cbSocketDataReceived)(String jsonStr) = NULL;
static void (*cbSocketConnection)(bool status) = NULL;
void initSocketIO(char *szDeviceNo,void (*ptr)(String jsonStr), void (*ptr2)(bool status))
{
    _ntpClient.begin();
    _socketIO.onEvent(onEvent);
    _socketIO.setReconnectInterval(10000);

    char szBuf[256];
    sprintf(szBuf, "%s?sn=%s", _SIO_PATH_, szDeviceNo);
    _socketIO.begin(SERVER_URL, SERVER_PORT, szBuf);

    cbSocketDataReceived = ptr;
    cbSocketConnection = ptr2;
}
/*----------------------------------------------------------------------
cmd from user via platform
example  : ["app-cmd",{"cmd":"output","content":{"field":3,"value":0}}]
----------------------------------------------------------------------*/
// void processEvent(String jsonStr)
// {
//     if(cbSocketDataReceived) (*cbSocketDataReceived)(jsonStr);
//     return;
//     const int capacity = JSON_ARRAY_SIZE(3) + 4 * JSON_OBJECT_SIZE(4) + 10 * JSON_OBJECT_SIZE(1);
//     StaticJsonDocument<capacity> doc;
//     DeserializationError err = deserializeJson(doc, jsonStr);
//     if (err)
//     {
//         debug_out2("deserializeJson() returned ", (const char *)err.f_str());
//         return;
//     }
//     pubStatus((char *)jsonStr.c_str()); // comment or not

//     auto cmd = doc["cmd"].as<const char *>();
//     if (strcmp(cmd, "output") == 0)
//     {
// #ifdef _IO_CONTROL_
//         auto f = doc["content"]["field"].as<int>();
//         auto v = doc["content"]["value"].as<int>();
//         setOutput(f, v);
//         setPublish(true);
// #endif
//     }
//     else if (strcmp(cmd, "output-all") == 0)
//     {
// #ifdef _IO_CONTROL_
//         auto v = doc["content"]["value"].as<int>();
//         setOutputAll(v);
//         setPublish(true);
// #endif
//     }
//     else if (strcmp(cmd, "reboot") == 0)
//     {
//         _socketIO.send(sIOtype_DISCONNECT, "");
//         reboot();
//     }
//     else if (strcmp(cmd, "sync") == 0)
//     {
//         setPublish(true);
//     }
//     else if (strcmp(cmd, "chat") == 0)
//     {
//         debug_out2("do chat", "");
//     }
// }
//-------------------------------------------------------------
void onEvent(const socketIOmessageType_t &type, uint8_t *payload, const size_t &length)
{
    switch (type)
    {
    case sIOtype_DISCONNECT:
        debug_out2("[IOc]", "Disconnected");
        _bSocketIOConnected = false;
        (*cbSocketConnection)(0);
        break;
    case sIOtype_CONNECT:
        debug_out2("[IOc] Connected to url: ", (const char *)payload);

        if (_bInitialConnection)
        {
            _bInitialConnection = false;
            // pubStatus("System Ready");
        }
        (*cbSocketConnection)(1);
        _bSocketIOConnected = true;
        break;
    case sIOtype_EVENT:
        debug_out2("[IOc] Got event: ", (char *)payload);
        {
            // payload like : ["app-cmd",{"cmd":"sync","content":""}]
            if (payload[2] != 'a')
                return; // only for app-cmd
            String text = ((const char *)&payload[0]);
            // processEvent(text.substring(text.indexOf('{'), text.length() - 1));
            if (cbSocketDataReceived)
                (*cbSocketDataReceived)(text.substring(text.indexOf('{'), text.length() - 1));
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
    //_bPublishRequired = false;

    DynamicJsonDocument doc(1024);
    JsonArray root = doc.to<JsonArray>();
    root.add(szEventName);

    JsonObject jsonObj = root.createNestedObject();

    if (szContent[0] == '[') // data begins with '[' 
        jsonObj["content"] = serialized(szContent);
    else
        jsonObj["content"] = szContent;

    _ntpClient.update(); // update time value
    // set seconds to millisecond value, require 64bit
    jsonObj["createdAt"] = (uint64_t)_ntpClient.getEpochTime() * 1000; 

    String output;
    serializeJson(doc, output); // JSON to String (serializion)
    _socketIO.sendEVENT(output);

    debug_out2("Pub: ", output.c_str());
}
//-------------------------------------------------------------
void stopSocketIO()
{
    _socketIO.send(sIOtype_DISCONNECT, "");
}

//-------------------------------------------------------------
void socketIOLoop()
{
    _socketIO.loop();
}
