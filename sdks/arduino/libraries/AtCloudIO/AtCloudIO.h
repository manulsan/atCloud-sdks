/*******************************************
  atCloudIO : based on SocketIOclient_Generic from arduino opensource
Updats
   1.0  : basic connection tests
******************************************/
#pragma once

#ifndef ATCLOUDIO_H
#define ATCLOUDIO_H

#include <Arduino.h>
//------------------------------------------------
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <Hash.h>
       
#ifndef SOCKET_IO_CLIENT_GENERIC_H_
  #include <SocketIOclient_Generic.h>
#endif

#define USE_SSL true
#ifdef USE_SSL
#define SERVER_PORT 9000 // to nginX https
#else
#define SERVER_PORT 9011
#endif

#define _WEBSOCKETS_LOGLEVEL_ 0   // 0-4
#define _SIO_PATH_ "/api/dev/io/" // do not change
#define SERVER_URL "damosys.com"

// void debug_out1(const char *sz1)
// {
//   Serial.println(sz1);
// }
// void debug_out2(const char *sz1, const char *sz2)
// {
//   Serial.print(sz1);
//   Serial.print(": ");
//   Serial.println(sz2);
// }
//------------------------------------------------
WiFiUDP ntpUDP;
NTPClient _ntpClient(ntpUDP, "asia.pool.ntp.org", 0, 600000); // https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/
//SocketIOclient _socketIO;

//class AtCloudIO : public SocketIOclient
class AtCloudIO : protected SocketIOclient
{
//   #ifdef __AVR__
//     typedef void (*SocketIOclientEvent)(socketIOmessageType_t type, uint8_t * payload, size_t length);
// #else
//     typedef std::function<void(socketIOmessageType_t type, uint8_t * payload, size_t length)> SocketIOclientEvent;
// #endif

//  using DataEvent = void (*)(String jsonStr);              // type aliasing
//  using ConnectionEvent = void (*)(bool connectionStatus); // type aliasing
private:
  byte pin;
  char sn[32 + 1];
  static bool bSocketIOConnected;

public:
  AtCloudIO(char *szSerialNo);
  //void init(DataEvent cbData, ConnectionEvent cbConnection);
  void init();
  bool publishData(char *szContent);
  bool publishStatus(char *szContent);
  void loop();
  void disConnect();

  virtual void onEvent(SocketIOclientEvent cbEvent);
private:
//  DataEvent dataEvent;
//  ConnectionEvent connectionEvent;
  //static void onEvent(const socketIOmessageType_t &type, uint8_t *payload, const size_t &length);
  //void onEvent(const socketIOmessageType_t &type, uint8_t *payload, const size_t &length);
};

#endif