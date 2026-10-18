#include "fetafsbr.h"
#include "pages.h"

bool wificlicheck=false;
AsyncWebServer server(80);
//DNSServer dnsServer;
AsyncWebSocket ws("/ws"); 

String webpage;
fsb_log_paths_t fsb_log_paths = {"core.log","error.log","user.log","wifi.log"};

void fsb_appendLogLine(const char* filename, const String& line, size_t maxSize) {
  String path = "/logs/" + String(filename);
  String backup = path + ".1";
  if (LittleFS.exists(path)) {
    File f = LittleFS.open(path, "r");
    if (f && f.size() > maxSize) {
      f.close();
      if (LittleFS.exists(backup)) LittleFS.remove(backup);
      LittleFS.rename(path, backup);
    } else f.close();
  }
  File f = LittleFS.open(path, "a");
  if (f) { f.println(String(millis()) + "\t" + line); f.close(); }
}

bool saveJsonToConfig(String filename, JsonDocument& doc) {
  File file = LittleFS.open("/config/" + filename, FILE_WRITE);
  if (!file) { fsb_appendLogLine(fsb_log_paths.error, "Config save failed: " + filename); return false; }
  serializeJson(doc, file); file.close(); return true;
}

bool loadJsonFromConfig(String filename, JsonDocument& doc) {
  File file = LittleFS.open("/config/" + filename, FILE_READ);
  if (!file) return false;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) fsb_appendLogLine(fsb_log_paths.error, "JSON parse error: " + filename);
  return true;
}

void wifiTask(void*) {
  unsigned long lastTry = 19000;
  const uint32_t interval = 20000; 
  for (;;) {
    if (WiFi.softAPgetStationNum() > 0) { vTaskDelay(pdMS_TO_TICKS(3000));  continue;}
    if (WiFi.status() == WL_CONNECTED) { vTaskDelay(pdMS_TO_TICKS(5000)); continue; }
    if (millis() - lastTry < interval) { vTaskDelay(pdMS_TO_TICKS(3000)); continue; }
    lastTry = millis();
    StaticJsonDocument<256> doc;
    if (!loadJsonFromConfig("wifi_config.json", doc)) {vTaskDelay(pdMS_TO_TICKS(5000)); continue; }
	String ssid = doc["ssid"] | "";
    String pass = doc["pass"] | "";
	wificlicheck = doc["enble"] | false;
	if(wificlicheck){
      if (!ssid.length()) {vTaskDelay( pdMS_TO_TICKS(5000)); continue; }
      WiFi.begin(ssid.c_str(), pass.c_str());
      vTaskDelay(pdMS_TO_TICKS(5000));  
    }
	vTaskDelay(pdMS_TO_TICKS(1000));
  }
}



String ConvBinUnits(int bytes, int resolution) {
  if(bytes < 1024) return String(bytes) + " B";
  else if (bytes < 1024 * 1024) return String((bytes / 1024.0), resolution) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String((bytes / 1024.0 / 1024.0), resolution) + " MB";
  else return "";
}

String getContentType(String filenametype) {
  if (filenametype == "download") return "application/octet-stream";
  else if (filenametype.endsWith(".txt")) return "text/plain";
  else if (filenametype.endsWith(".html")) return "text/html";
  else if (filenametype.endsWith(".css")) return "text/css";
  else if (filenametype.endsWith(".js")) return "application/javascript";
  else if (filenametype.endsWith(".png")) return "image/png";
  else if (filenametype.endsWith(".jpg")) return "image/jpeg";
  else if (filenametype.endsWith(".json")) return "application/json";
  else return "text/plain";
}

String rootserch(const String &rtt, const String &color, unsigned int &numfiles, const String &label){
  String page;
  page.reserve(1024);
  page += "<details style='margin-top:2px;'><summary style='font-size:16px; padding:10px; background:#e0e0e0; border-radius:4px; cursor:pointer;'>" + label + "</summary>";
  File root = LittleFS.open(rtt);
  if (root && root.isDirectory()) {
    root.rewindDirectory();
    File file = root.openNextFile();
    while (file) {
      String fname1 = rtt + (String(file.name()).startsWith("/") ? String(file.name()).substring(1) : file.name());
      page += "<div class='container' style='color:" + color + "'>";
      page += "<div class='cell'>" + fname1 + "</div>";
      page += "<div class='cell'>" + ConvBinUnits(file.size(), 2) + "</div>";
      page += "<div class='cell'>";
      page += "<a href='/viewhandler~/" + fname1 + "' target='_blank' class='viewbtn'>View </a>";
      page += "<a href='/downloadhandler~/" + fname1 + "' class='downloadbtn'>Download </a>";
      page += "<a href='/deletehandler~/" + fname1 + "' onclick=\"return confirm('Are you sure?');\"  class='deletebtn'>Delete </a></div>";
      page += "</div>";
      file = root.openNextFile();
      numfiles++;
    }
    root.close();
  }
  page += "</details>";
  return page;
}

void dirpg(bool admin){
  unsigned int numfiles=0;
  webpage = FPSTR(DASHBOARD_HEAD); 

  if(admin){
    webpage += rootserch("/config/","black",numfiles,"Config Files");
    webpage += rootserch("/www/","blue",numfiles,"Web Files");
    webpage += rootserch("/logs/","brown",numfiles,"Log Files");
  }
  webpage += rootserch("/data/","green",numfiles,"Data Files");
  webpage += "</div>";

  String formBlock = FPSTR(FORM_BLOCKS);
  String ssidStr = "" , passStr = "", enble="";
  StaticJsonDocument<256> doc;
  if (loadJsonFromConfig("wifi_config.json", doc)) {
    ssidStr = doc["ssid"] | "";
    passStr = doc["pass"] | "";
    enble = (doc["enble"].as<bool>()==1)?"checked":"";
    wificlicheck= enble;
  }
  formBlock.replace("%SSID%", ssidStr);
  formBlock.replace("%PASS%", passStr);
  formBlock.replace("%WIENBL%", enble);
  if(admin) webpage += formBlock;

  size_t total = LittleFS.totalBytes();
  size_t used  = LittleFS.usedBytes();
  int percent  = (total > 0) ? (100 * used / total) : 0;

  String params = FPSTR(PARAMS_BLOCK);
  params.replace("%USED%", String(percent));
  params.replace("%USED_STR%", ConvBinUnits(used, 2));
  params.replace("%TOTAL_STR%", ConvBinUnits(total, 2));
  params.replace("%NUMFILES%", (numfiles == 0) ? "Pending Dir or Empty" : String(numfiles));
  params.replace("%IP%", (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "Unknown");
  params.replace("%RSSI%", (WiFi.status() == WL_CONNECTED) ? String(WiFi.RSSI()) : "N/A");
  
  unsigned long ms = millis();
  unsigned long days    = ms / (1000UL * 60UL * 60UL * 24UL);
  unsigned long hours   = (ms / (1000UL * 60UL * 60UL)) % 24;
  unsigned long minutes = (ms / (1000UL * 60UL)) % 60;
  unsigned long seconds = (ms / 1000UL) % 60;
  String uptimeStr = String(days) + "d " +
                    String(hours) + "h " +
                    String(minutes) + "m " +
                    String(seconds) + "s";
  params.replace("%UPTIME%", uptimeStr);

  webpage += params;
  webpage += "<center><footer><p><i>Copyright &copy;&nbsp;ARAZco 2025 Version 2.9 </i></p></footer></center>";
  webpage += "</body></html>";
}

void initfsb(String apname , String appass ,bool dnsEnabled=false){
  webpage.reserve(6000); 
  if(!LittleFS.begin(true)){}
  if(!LittleFS.exists("/www")) LittleFS.mkdir("/www");
  if(!LittleFS.exists("/data")) LittleFS.mkdir("/data");
  if(!LittleFS.exists("/config")) LittleFS.mkdir("/config");
  if(!LittleFS.exists("/logs")) LittleFS.mkdir("/logs");

  WiFi.mode(WIFI_AP_STA); 
  WiFi.setSleep(false); 
  WiFi.setHostname(apname.c_str()); 
  WiFi.softAP(apname , appass);
  WiFi.softAPsetHostname(apname.c_str());   
  

  server.on("/admin", HTTP_GET, [](AsyncWebServerRequest * request) {  
    if(!request->authenticate("admin", "abc123456"))return request->requestAuthentication();            
    dirpg(true); 
    request->send(200, "text/html", webpage);
  });

  server.on("/files", HTTP_GET, [](AsyncWebServerRequest * request) {              
    dirpg(false); 
    request->send(200, "text/html", webpage);
  });

  server.on("/handleupdate", HTTP_POST, [](AsyncWebServerRequest * request) {},[](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data,size_t len, bool final) {
    String logmessage;
    if (!index) {
      logmessage = "OTA Update Start: " + String(filename);
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {fsb_appendLogLine(fsb_log_paths.error, "OTA begin failed");} 
    }

    if (len) {
     if (Update.write(data, len) != len) { fsb_appendLogLine(fsb_log_paths.error, "OTA write failed"); }      
    }

    if (final) {
      if (Update.end(true)) {
        request->redirect("/admin");
	    vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP.restart();
      }
      else{
        fsb_appendLogLine(fsb_log_paths.error, "OTA end failed");
      }
    }
  });

  server.on("/handleupload", HTTP_POST, [](AsyncWebServerRequest * request) {},[](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      String folder = request->getParam("folder", true)->value();
      if (folder != "www" && folder != "data" && folder != "config") folder = "data"; 
      String path = "/" + folder + "/" + filename;
      request->_tempFile = LittleFS.open(path, FILE_WRITE);
    }
    if (request->_tempFile) {
      if (len) request->_tempFile.write(data, len);
      if (final) { request->_tempFile.close(); request->redirect("/admin"); }
    }
    else{
       fsb_appendLogLine(fsb_log_paths.error, "Upload failed, cannot open file: " + filename);
    }
  });

  server.on("/viewhandler~", HTTP_GET, [](AsyncWebServerRequest *request) {
    String filename = request->url().substring(request->url().indexOf("~/") + 1);
    File file = LittleFS.open(filename, "r");
    if (!file || file.isDirectory()) {
      fsb_appendLogLine(fsb_log_paths.error, "File open failed: " + filename);
      request->send(404, "text/plain", "File not found or is a directory");
      return;
    }
    String contentType = getContentType(filename);
    request->send(LittleFS, filename, contentType);
  });

  server.on("/downloadhandler~", HTTP_GET, [](AsyncWebServerRequest *request){
    String filename = request->url().substring(request->url().indexOf("~/") + 1);
    if (!LittleFS.exists(filename)) {
      fsb_appendLogLine(fsb_log_paths.error, "Download failed, file not found: " + filename);
      request->send(404, "text/plain", "File not found");
      return;
    }
    request->send(LittleFS, filename, "application/octet-stream");
  });

  server.on("/deletehandler~",HTTP_GET, [](AsyncWebServerRequest * request){
     String filename = request->url().substring(request->url().indexOf("~/") + 1);
     if (!LittleFS.remove(filename)){ fsb_appendLogLine(fsb_log_paths.error, "Delete failed: " + filename);}
     request->redirect("/admin");
  });
	
  server.on("/setwifi", HTTP_POST, [](AsyncWebServerRequest *request){
    String ssid = request->getParam("ssid", true)->value();
    String pass = request->getParam("pass", true)->value();
    bool enble = false;
    if (request->hasParam("wienbl", true)) {
        String enbleStr = request->getParam("wienbl", true)->value();
        enble = (enbleStr == "on" || enbleStr == "true" || enbleStr == "1");
    }
    StaticJsonDocument<256> doc;
    doc["ssid"] = ssid; doc["pass"] = pass; doc["enble"] = enble;
    saveJsonToConfig("wifi_config.json", doc);
    request->redirect("/admin");
  });
  
  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){
    fsb_appendLogLine(fsb_log_paths.core, "Manual reset triggered");
    request->send(200, "text/plain", "Restarting...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP.restart();
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect("/admin"); 
  });

  //if(dnsEnabled){dnsServer.start(53, "*", WiFi.softAPIP());}
}


void runServer(bool SocketEnabled=false){
  xTaskCreatePinnedToCore(wifiTask,"wifiTask",8192,nullptr,1,nullptr,1);
  fsb_appendLogLine(fsb_log_paths.core, "Program Init Started successfully");
  server.begin();
  
  if(SocketEnabled){
	  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,AwsEventType type, void *arg, uint8_t *data, size_t len) {
		if (type == WS_EVT_CONNECT) {}
	  });
	  server.addHandler(&ws);
  }
}