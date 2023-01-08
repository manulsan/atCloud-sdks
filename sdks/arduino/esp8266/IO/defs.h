#if !defined(ESP8266)
#error This code is intended to run only on the ESP8266 boards ! Please check your Tools->Board setting.
#endif

#ifndef DEFS_H
#define DEFS_H
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>

#include <SocketIOclient_Generic.h>
#include <Hash.h>

WiFiUDP ntpUDP;
ESP8266WiFiMulti WiFiMulti;
NTPClient _ntpClient(ntpUDP, "asia.pool.ntp.org", 0, 600000); // https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/

SocketIOclient _socketIO;

#define SERVER_URL "damosys.com"
#define USE_SSL true
#ifdef USE_SSL
#define SERVER_PORT 9000 // to nginX https
#else
#define SERVER_PORT 9011
#endif

#define DATA_EVENT 0
#define STATUS_EVENT 1

#define DEBUG

#ifdef DEBUG
void debugHexDump(char *szTitle, uint8_t *payload, const size_t &length)
{
  Serial.print(szTitle);
  hexdump(payload, length);
}
#include <stdarg.h>
void debugF(char *fmt, ...)
{
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start(args, fmt);
#ifdef __AVR__
  vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); // progmem for AVR
#else
  vsnprintf(buf, sizeof(buf), (const char *)fmt, args); // for the rest of the world
#endif
  va_end(args);
  Serial.print(buf);
}
#else
#define debugHexDump(szTitle, payload, length) ((void)0)
//#define debugF(char *fmt, ...) // ((void)0)
#define debugF(fmt, ...) // ((void)0)
#endif
#endif