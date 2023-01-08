// https://roboticsbackend.com/arduino-create-library/
#include "AtCloudIO.h"
AtCloudIO::AtCloudIO(char *szSerialNo)
{
  // this->pin = pin;
  // init();
  memset(this->sn, 0x00, sizeof(this->sn));
  strncpy(this->sn, szSerialNo, 32);

  bSocketIOConnected = false;
  //debug_out2("AtCloudIO::AtCloudIO sn=", this->sn);
}

//void AtCloudIO::init(DataEvent cbData, ConnectionEvent cbConnection)
void AtCloudIO::init()
{
  // dataEvent = cbData;
  // connectionEvent = cbConnection;
  _ntpClient.begin();

  //_socketIO.onEvent(onEvent);

  char szBuf[256];
  sprintf(szBuf, "%s?sn=%s", _SIO_PATH_, this->sn);
  // this->dataEvent = NULL;
  // this->connectionEvent = NULL;

#ifdef USE_SSL
  //_socketIO.beginSSL(SERVER_URL, SERVER_PORT, szBuf);
  beginSSL(SERVER_URL, SERVER_PORT, szBuf);
#else
  //_socketIO.begin(SERVER_URL, SERVER_PORT, szBuf);
  begin(SERVER_URL, SERVER_PORT, szBuf);
  
#endif
}
////////////////////////////////////////

/**
   set callback function
   @param cbEvent SocketIOclientEvent
*/
void AtCloudIO::onEvent(SocketIOclientEvent cbEvent)
{
  _cbEvent = cbEvent;
}
void AtCloudIO::loop()
{
  // Serial.printf("publishStatus");
  //_socketIO.loop();
  loop();
}
bool AtCloudIO::publishData(char *szContent)
{
  Serial.printf("publishData:\n");
  Serial.printf(szContent);
  return true;
}
bool AtCloudIO::publishStatus(char *szContent)
{
  Serial.printf("publishStatus");
  return true;
}

void AtCloudIO::disConnect()
{
  //_socketIO.send(sIOtype_DISCONNECT, "");
  send(sIOtype_DISCONNECT, "");

}

// void AtCloudIO::onEvent(const socketIOmessageType_t &type, uint8_t *payload, const size_t &length)
// {
//   switch (type)
//   {
//   case sIOtype_DISCONNECT:
//     //debug_out2("[IOc]", "Disconnected");
//     //this->bSocketIOConnected = false;
//     bSocketIOConnected = false;
//     //(*cbSocketConnection)(0);
//     //connectionEvent(0); //
//     this->connectionEvent(0); //
//     break;
//   case sIOtype_CONNECT:
//     //debug_out2("[IOc] Connected to url: ", (const char *)payload);

//     // if (_bInitialConnection)
//     {
//       //_bInitialConnection = false;
//       // pubStatus("System Ready");
//     }
//     //(*cbSocketConnection)(1);
//      this->connectionEvent(1); //
//     //connectionEvent(1); //
//     bSocketIOConnected = true;
//     break;
//   case sIOtype_EVENT:
//     //debug_out2("[IOc] Got event: ", (char *)payload);
//     {
//       // payload like : ["app-cmd",{"cmd":"sync","content":""}]
//       if (payload[2] != 'a')
//         return; // only for app-cmd
//       String text = ((const char *)&payload[0]);
//       // processEvent(text.substring(text.indexOf('{'), text.length() - 1));
//       if (dataEvent)
//       {
//         //(*cbSocketDataReceived)
//         dataEvent(text.substring(text.indexOf('{'), text.length() - 1));
//       }
//     }
//     break;
//   case sIOtype_ACK:
//     // debugHexDump("[IOc] Get ack: ", payload, length);
//     break;
//   case sIOtype_ERROR:
//     // debugHexDump("[IOc] Get error: ", payload, length);
//     break;
//   case sIOtype_BINARY_EVENT:
//     // debugHexDump("[IOc] Get binary: ", payload, length);
//     break;
//   case sIOtype_BINARY_ACK:
//     // debugHexDump("[IOc] Get binary ack: ", payload, length);
//     break;
//   case sIOtype_PING:
//     // debug_out2("[IOc]", "Got PING");
//     break;
//   case sIOtype_PONG:
//     // debug_out2("[IOc]", "Got PONG");
//     break;
//   default:
//     break;
//   }
// }