#include <Wlan.h>
#include <ESPmDNS.h>
#include "Stats.h"
#include "Logs.h"
#include "Clock.h"

static Wlan* instance = nullptr;

String username = "admin";
String userPSW = "";
String adminName = "admin";
String adminPSW = "";
vector<SessionToken> sessionTokens;

struct WifiTaskData {
    Wlan* self;
    long ttlMs;
};


Wlan::Wlan() : server(80), wifiHandler(NULL), startSSID(""), startPWD(""), active(false), ttl(0){
    instance = this; 
}

Wlan::~Wlan() {
    endWifi();
}

// Getter
bool Wlan::isActive() const{
    return active;
}



void Wlan::init() {
    startSSID = ConfigManager::getInstance()->getWifiSSID();
    startPWD = ConfigManager::getInstance()->getWifiPassword();
    username = ConfigManager::getInstance()->getUserName();
    userPSW = ConfigManager::getInstance()->getUserPassword();
    adminName = ConfigManager::getInstance()->getAdminOverrideName();
    adminPSW = ConfigManager::getInstance()->getAdminOverridePassword();
    ConfigManager::getInstance()->addObserver(instance);
}

int retrys = 0;
int maxRetrys = 2;
int maxTimeout = 5;
bool Wlan::startWifi(long ttlMs) {
    bool returnValue = false;
    retrys = 0; 

    while(retrys <= maxRetrys) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(startSSID.c_str(), startPWD.c_str());
        Logs::getInstance()->addLog("Connecting to WiFi");

        int timeout = 0;
        while (WiFi.status() != WL_CONNECTED && timeout < maxTimeout * 2) {
            delay(500);
            Logs::getInstance()->addLog(".", false);
            timeout++;
        }

        if(WiFi.status() == WL_CONNECTED){
            Logs::getInstance()->addLog("Connected to WiFi!");
            Stats::getInstance()->setWifiStatus(true);
            Stats::getInstance()->setLastConnection(Clock::getInstance()->getTime());
            returnValue = true;
            break;
        }

        retrys++;
        Logs::getInstance()->addLog("Retry failed...");
    }

    if(!returnValue){
        Logs::getInstance()->addLog("Failed to connect to WiFi, starting AP...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP("ESP_Test", "123456789");
    }

    if (!MDNS.begin("esp32")) {  // hostname: esp32.local
        Serial.println("Error setting up MDNS");
    } else {
        Serial.println("MDNS responder started: http://esp32.local");
    }

    const char* headersToCollect[] = {"Cookie", "Host", "User-Agent", "Accept"};
    
    // Weise den Server an, diese Header zu sammeln
    // Die '4' muss der Anzahl der Header im Array entsprechen
    server.collectHeaders(headersToCollect, 4);

    // register server routes
    server.on("/", [](){ instance->handleRoot(); });
    server.on("/index.html", [](){ instance->handleRoot(); });

    server.on("/login.html", [](){ instance->handleLogin(); });
    server.on("/loginConfirmation", HTTP_POST, [](){ 
        String recievedUserName = instance->server.arg("userName");
        String recievedUserPsw = instance->server.arg("userPSW");

        Logs::getInstance()->addLog("Login attempt-> recUser:" + recievedUserName + " recPwd:"+ recievedUserPsw + " corrUser:" + username + " corrPwd:" + userPSW);

        if(!(recievedUserName.equals(username) || recievedUserName.equals(adminName)) || !(recievedUserPsw.equals(userPSW) || recievedUserPsw.equals(adminPSW))){
            instance->server.send(404, "text/plain", "Invalid Login info!");
            return;
        }

        SessionToken token;
        token.newToken();
        sessionTokens.push_back(token);
        
        instance->server.sendHeader("Set-Cookie", "session=" + token.getToken() + "; Path=/; Max-Age=600");
        instance->server.sendHeader("Location", "/login.html");
        instance->server.send(302, "text/plain", "");
        
    });
    
    server.on("/dev.html", [](){ instance->handleDev(); });
    server.on("/changeUserInfos", HTTP_POST, [](){ 
        String recievedUserName = instance->server.arg("newUserName");
        String recievedUserPsw = instance->server.arg("newUserPWD");

        ConfigManager::getInstance()->setUserName(recievedUserName);
        ConfigManager::getInstance()->setUserPassword(recievedUserPsw);

        Serial.print("new Credentials: " + recievedUserName + recievedUserPsw);

        sessionTokens.clear();
        
        instance->server.sendHeader("Location", "/dev.html");
        instance->server.send(302, "text/plain", "");
    });
    
    server.on("/complete.html", HTTP_POST, [](){ instance->handleSubmit(); }, [](){ instance->handleFileUpload(); });
    server.on("/complete.html", HTTP_GET, [](){
        File f = LittleFS.open("/complete.html", "r");
        if(!f){
            instance->server.send(404, "text/plain", "File not found");
            return;
        }
        instance->server.streamFile(f, "text/html");
        f.close();
    });

    server.on("/api/logs", HTTP_GET, [](){
        String logs = Logs::getInstance()->getLogs();
        instance->server.send(200, "text/plain", logs);
    });
    server.on("/api/infos", HTTP_GET, [](){
        String infos = "";

        uint64_t totalMs = Stats::getInstance()->getUptime();
        uint64_t totalSec = totalMs / 1000;

        uint64_t days = totalSec / 86400;             // 24*60*60
        uint64_t hours = (totalSec % 86400) / 3600;   // Reststunden
        uint64_t minutes = (totalSec % 3600) / 60;    // Restminuten
        uint64_t seconds = totalSec % 60;             // Restsekunden

        String stringUptime = String(days) + "d " 
                            + String(hours) + "h " 
                            + String(minutes) + "m " 
                            + String(seconds) + "s";

        infos += "UPTIME=" + stringUptime + "; ";
        String wifiState = "WIFISTATUS=TTl:"+  String(instance->ttl) + "; ";
        String IP = "IPADDRESS=" + WiFi.localIP().toString() + "; ";
        infos += wifiState;
        infos += IP;

        infos += "LAST_CONNECTION=" + Stats::getInstance()->getLastConnection().toString() + "; ";
        infos += "LAST_API_CALL=" + Stats::getInstance()->getLastApiCall().toString() + "; ";

        instance->server.send(200, "text/plain", infos);
    });
    
    
    server.serveStatic("/style.css", LittleFS, "/style.css");
    server.serveStatic("/script.js", LittleFS, "/script.js");
    
    //server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.begin();
    active = true;
    stopTask = false;
    
    ttl = ttlMs;
    
    WifiTaskData* data = new WifiTaskData();
    data->self = this;
    data->ttlMs = ttlMs; 
    
    // start task on Core 1
    xTaskCreatePinnedToCore(wifiTaskWrapper, "wifi", 8192, data, 1, &wifiHandler, 0);
    
    if(WiFi.status() == WL_CONNECTED){
        // Stats::getInstance()->setWifiStatus(true);
        // Timestamp t = Clock::getInstance()->getTime();
        // Stats::getInstance()->setLastConnection(t);
        Logs::getInstance()->addLog("WiFi started");
    }
    return returnValue;
}


void Wlan::endWifi() {
    if(wifiHandler != NULL) { 
        Logs::getInstance()->addLog("Wifi stop requested"); 
        retrys = 0;
        stopTask = true; 
    }
}

void Wlan::wifiTaskWrapper(void* param) {
    WifiTaskData* data = (WifiTaskData*)param;
    Wlan* self = data->self;
    long ttl = data->ttlMs;

    unsigned long last = millis();

    while (1) {
        self->server.handleClient();

        unsigned long now = millis();
        ttl -= (now - last);
        last = now;

        if (ttl <= 0 || self->stopTask) {
            if (ttl <= 0) {
                Logs::getInstance()->addLog("Wifi TTL expired");
            }
            break;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    
    Logs::getInstance()->addLog("Wifi task shutting down...");
    
    Stats::getInstance()->setWifiStatus(false);
    self->server.close();
    WiFi.softAPdisconnect(true);
    self->active = false;
    self->wifiHandler = NULL;
    sessionTokens.clear();
    
    delete data;

    vTaskDelete(NULL); 
}


void Wlan::handleRoot() {
    File file = LittleFS.open("/index.html", "r");
    if(!file){
        server.send(404, "text/plain", "File not found");
        return;
    }

    String html = "";
    while(file.available()){
        html += char(file.read());
    }

    html.replace("{SSID}", ConfigManager::getInstance()->getWifiSSID());
    html.replace("{SSIDPWD}", ConfigManager::getInstance()->getWifiPassword());

    html.replace("{AUOTTIME_SELECTED}", ConfigManager::getInstance()->getAutoTime() ? "checked" : "");
    html.replace("{TIME}", ConfigManager::getInstance()->getTime());
    html.replace("{DATE}", ConfigManager::getInstance()->getDate());
    
    html.replace("{DISPLAY_OPTIONS_TIME}", ConfigManager::getInstance()->getShowTime() ? "checked" : "");
    html.replace("{DISPLAY_OPTIONS_WEATHER}", ConfigManager::getInstance()->getShowWeather() ? "checked" : "");
    html.replace("{DISPLAY_OPTIONS_DATE}", ConfigManager::getInstance()->getShowDate() ? "checked" : "");

    //server.streamFile(file, "text/html");
    file.close();
    server.send(200, "text/html", html);
}

void Wlan::handleDev() {
    // Serial.println("===== HEADER DUMP =====");
    // for (int i = 0; i < server.headers(); i++) {
    //     Serial.println(server.headerName(i) + " = " + server.header(i));
    // }
    // Serial.println("========================");

    String cookie = server.header("Cookie");

    if(!validateToken(cookie)){
        server.sendHeader("Location", "/login.html");
        server.send(302, "text/plain", "");
        Logs::getInstance()->addLog("invalid Session redirecting!");
        return;
    }

    File file = LittleFS.open("/dev.html", "r");
    if(!file){
        server.send(404, "text/plain", "File not found");
        return;
    }
    
    String html;
    while(file.available()){
        html += char(file.read());
    }
    //server.streamFile(file, "text/html");
    file.close();
    
    html.replace("{USERNAME}", username);
    html.replace("{USERPSW}", userPSW);
    
    server.send(200, "text/html", html);


}

void Wlan::handleLogin() {
    String cookie = server.header("Cookie");
    // int asd = server.headers();
    // for(int i = 0; i < asd; i++){
    //     String name = String(i) + ". " + server.headerName(i);
    //     Serial.println(name);
    // }

    // Serial.println("login cookies-> " + cookie);

    if(validateToken(cookie)){
        server.sendHeader("Location", "/dev.html");
        server.send(302, "text/plain", "");
        return;
    }

    File file = LittleFS.open("/login.html", "r");
    if(!file){
        server.send(404, "text/plain", "File not found");
        return;
    }
    server.streamFile(file, "text/html");
    file.close();
}

void Wlan::handleSubmit() {
    File file = LittleFS.open("/complete.html", "r");
    if(file){
        server.streamFile(file, "text/html");
        file.close();
    }

	String ssid = server.arg("ssid");
	String pass = server.arg("ssidPwd");
    if(!ssid.isEmpty() && !pass.isEmpty()){
        ConfigManager::getInstance()->setWifiSSID(ssid);
        ConfigManager::getInstance()->setWifiPassword(pass);
    }
	
	bool autoTime = server.hasArg("autoTime");
    ConfigManager::getInstance()->setAutoTime(autoTime);
	if(!autoTime){
		String timeString = server.arg("time");
		int hour = timeString.substring(0,2).toInt(); 
		int min = timeString.substring(3,5).toInt(); 
        ConfigManager::getInstance()->setTime(timeString);
		
		String dateString = server.arg("date");
		int year = dateString.substring(0,4).toInt();
		int month = dateString.substring(5,7).toInt();
		int day = dateString.substring(8,10).toInt();
        ConfigManager::getInstance()->setDate(dateString);
	}
	
	Serial.println("Received WiFi credentials:");
	Serial.println(ssid);
	Serial.println(pass);
}

void Wlan::handleFileUpload() {
    // handle uploaded file if needed
}


bool Wlan::validateToken(String cookieHeader){
    int pos = cookieHeader.indexOf("session=");
    if(pos == -1){
        return false;
    }
    pos += 8;
    String sessionId = cookieHeader.substring(pos, pos + 30);

    for(int i = 0; i < sessionTokens.size(); i++){
        if(sessionTokens[i].isExpired()){
            sessionTokens.erase(sessionTokens.begin() + i);
            i--;
        }else{
            if(sessionTokens[i].getToken().equals(sessionId)){
                return true;
            }
        }
    }
    return false;
}
