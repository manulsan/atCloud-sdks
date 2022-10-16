#if !defined(ESP8266)
#error This code is intended to run only on the ESP8266 boards ! Please check your Tools->Board setting.
#endif

#ifndef DEFS_H
#define DEFS_H

// #ifdef ESP8266
// extern "C"
// {
// #include "user_interface.h"
// }
// #endif

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>

//#include <WebSocketsClient_Generic.h>
#include <SocketIOclient_Generic.h>
#include <Hash.h>

#define SERVER_URL "192.168.123.226"
#define SERVER_PORT 9011

WiFiUDP ntpUDP;
ESP8266WiFiMulti WiFiMulti;
NTPClient _ntpClient(ntpUDP, "asia.pool.ntp.org", 0, 600000); // https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/

SocketIOclient _socketIO;

//**********************************************************
// user can change below 2 lines
#define _IO_CONTROL_
#ifdef _IO_CONTROL_
#define _BULB_CONTROL_
#endif
#define DEBUG
//**********************************************************

#ifdef DEBUG
//----------------------------------------------------------------------
void debug_out1(const char *sz1)
{
  Serial.println(sz1);
}
void debug_out2(const char *sz1, const char *sz2)
{
  Serial.print(sz1);
  Serial.print(": ");
  Serial.println(sz2);
}
//----------------------------------------------------------------------
void debugHexDump(char *szTitle, uint8_t *payload, const size_t &length)
{
  Serial.print(szTitle);
  hexdump(payload, length);
}
#else
#define debug_out1(sz1)
#define debug_out2(sz1, sz2)
#define debugHexDump(szTitle, payload, length)
#endif
#endif