#ifndef FTF_BROWSER_H
#define FTF_BROWSER_H

#include "Arduino.h"
#include "WiFi.h"
#include "AsyncTCP.h"
#include "Update.h"
#include "ESPAsyncWebServer.h"
#include "FS.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
//#include <DNSServer.h>

struct fsb_log_paths_t {
  char core[16];
  char error[16];
  char user[16];
  char wifi[16];
};

extern AsyncWebServer server;
//extern DNSServer dnsServer;
extern AsyncWebSocket ws;
extern const char DASHBOARD_HEAD[] PROGMEM;
extern const char FORM_BLOCKS[] PROGMEM;
extern const char PARAMS_BLOCK[] PROGMEM;
extern fsb_log_paths_t fsb_log_paths;


void dirpg(bool admin=false);
void fsb_appendLogLine(const char* filename, const String& line, size_t maxSize = 10 * 1024);
bool saveJsonToConfig(String filename, JsonDocument& doc);
bool loadJsonFromConfig(String filename, JsonDocument& doc);

void wifiTask(void*);

String ConvBinUnits(int bytes, int resolution);
String getContentType(String filenametype);
String rootserch(const String &rtt, const String &color, unsigned int &numfiles, const String &label);
void dirpg(bool admin);
void initfsb(String apname , String appass ,bool dnsEnabled);
void runServer(bool SocketEnabled);

#endif