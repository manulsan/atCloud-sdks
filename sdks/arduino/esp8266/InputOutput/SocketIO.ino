#include "defs.h"
#define _WEBSOCKETS_LOGLEVEL_ 0   // 0-4
#define _SIO_PATH_ "/api/dev/io/" // do not change

bool _bSocketIOConnected = false;
bool _bInitialConnection = true;
static void (*cbSocketDataReceived)(String jsonStr) = NULL;
static void (*cbSocketConnection)(bool status) = NULL;
void initSocketIO(char *szDeviceNo, void (*ptr)(String jsonStr), void (*ptr2)(bool status))
{
    _ntpClient.begin();
    _socketIO.onEvent(onEvent);
    _socketIO.setReconnectInterval(10000);

    char szBuf[256];
    sprintf(szBuf, "%s?sn=%s", _SIO_PATH_, szDeviceNo);

#ifdef USE_SSL
        _socketIO.beginSSL(SERVER_URL, SERVER_PORT, szBuf);        
#else
        _socketIO.begin(SERVER_URL, SERVER_PORT, szBuf);
#endif
    cbSocketDataReceived = ptr;
    cbSocketConnection = ptr2;
}
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
//-------------------------------------------------------------
uint8_t publish(uint8_t eventType, char *szContent)
{
    if (!_bSocketIOConnected)
    {
        debug_out2(__FUNCTION__, "err: socket is not connected");
        return 1;
    }
    if (eventType != DATA_EVENT && eventType != STATUS_EVENT)
    {
        debug_out2(__FUNCTION__, "err: invalid eventType");
        return 2;
    }
    DynamicJsonDocument doc(1024);
    JsonArray root = doc.to<JsonArray>();
    root.add(eventType == DATA_EVENT ? "dev-data" : "dev-status");

    JsonObject jsonObj = root.createNestedObject();
    jsonObj["content"] = eventType == DATA_EVENT ? serialized(szContent) : szContent;
    _ntpClient.update();                                               // update time value
    jsonObj["createdAt"] = (uint64_t)_ntpClient.getEpochTime() * 1000; // set seconds to millisecond value, require 64bit

    String output;
    serializeJson(doc, output); // JSON to String (serializion)
    _socketIO.sendEVENT(output);

    debug_out2("Pub: ", output.c_str());
    return 0;
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
