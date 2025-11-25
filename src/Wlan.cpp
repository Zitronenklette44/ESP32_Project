#include <Wlan.h>
#include <ESPmDNS.h>
#include "Stats.h"
#include "Logs.h"

static Wlan* instance = nullptr;

String username = "admin";
String userPSW = "";
vector<SessionToken> sessionTokens;

struct WifiTaskData {
    Wlan* self;
    long ttlMs;
};


Wlan::Wlan(vector<String>& vals) : server(80), values(vals), wifiHandler(NULL), changedValues(false), startSSID(""), startPWD(""), active(false){
    instance = this; 
}

Wlan::~Wlan() {
    endWifi();
}

// Getter
bool Wlan::hasChangedValues() const {
    return changedValues;
}

bool Wlan::isActive() const{
    return active;
}



void Wlan::init() {
    values[0] = "ESP_Test";
    values[1] = "123456789";
}

void Wlan::startWifi( long ttlMs) {
    if(!values[0].equals("ESP_Test") && !values[1].equals("123456789")){
        // connect to existing WiFi
        WiFi.mode(WIFI_STA);
        WiFi.begin(values[0].c_str(), values[1].c_str());
        Serial.print("Connecting to WiFi");
        int timeout = 0;
        while (WiFi.status() != WL_CONNECTED && timeout < 20) { // 10s timeout
            delay(500);
            Serial.print(".");
            timeout++;
        }
        if(WiFi.status() == WL_CONNECTED){
            Serial.println();
            Serial.println("Connected to WiFi!");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println();
            Serial.println("Failed to connect to WiFi, starting AP...");
            WiFi.mode(WIFI_AP);
            WiFi.softAP("ESP_Test", "123456789");
        }
    } else {
        // default AP
        WiFi.mode(WIFI_AP);
        WiFi.softAP(values[0], values[1]);
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

        if(!recievedUserName.equals(username) || !recievedUserPsw.equals(userPSW)){
            instance->server.send(404, "text/plain", "Invalid Login info!");
            return;
        }

        SessionToken token;
        token.newToken();
        sessionTokens.push_back(token);
        
        instance->server.sendHeader("Set-Cookie", "session=" + token.getToken() + "; Path=/; Max-Age=3600");
        instance->server.sendHeader("Location", "/login.html");
        instance->server.send(302, "text/plain", "");
    });
    
    server.on("/dev.html", [](){ instance->handleDev(); });
    server.on("/changeUserInfos", HTTP_POST, [](){ 
        String recievedUserName = instance->server.arg("newUserName");
        String recievedUserPsw = instance->server.arg("newUserPWD");

        username = recievedUserName;
        userPSW = recievedUserPsw;

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
        String wifiState;
        if (WiFi.status() == WL_CONNECTED) {
            wifiState = "WIFISTATUS=true; IPADDRESS=" + WiFi.localIP().toString() + "; ";
        } else {
            wifiState = "WIFISTATUS=false; IPADDRESS=NULL; ";
        }
        infos += wifiState;

        infos += "LAST_CONNECTION=" + Stats::getInstance()->getLastConnection().toString() + "; ";
        infos += "LAST_API_CALL=" + Stats::getInstance()->getLastApiCall().toString() + "; ";

        instance->server.send(200, "text/plain", infos);
    });


    server.serveStatic("/style.css", LittleFS, "/style.css");
    server.serveStatic("/script.js", LittleFS, "/script.js");

    //server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.begin();
    active = true;

    WifiTaskData* data = new WifiTaskData();
    data->self = this;
    data->ttlMs = ttlMs; 

    // start task on Core 1
    xTaskCreatePinnedToCore(wifiTaskWrapper, "wifi", 4096, data, 1, &wifiHandler, 1);

    Stats::getInstance()->setWifiStatus(true);
    Timestamp t;
    t.second = 10;
    t.minute = 15;
    t.hour = 22;
    t.day = 17;
    t.month = 11;
    t.year = 2025;
    Stats::getInstance()->setLastConnection(t);
    Logs::getInstance()->addLog("WiFi started");
}

void Wlan::endWifi() {
    if(wifiHandler != NULL) {
        vTaskDelete(wifiHandler);
        wifiHandler = NULL;
    }
    Logs::getInstance()->addLog("Wifi ending");
    Stats::getInstance()->setWifiStatus(false);
    server.close();
    WiFi.softAPdisconnect(true);
    active = false;
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

        if (ttl <= 0) {
            delete data; 
            self->endWifi();
            //vTaskDelete(NULL);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
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

    html.replace("{SSID}", values[0]);
    html.replace("{SSIDPWD}", values[1]);

    html.replace("{AUOTTIME_SELECTED}", values[2]);
    html.replace("{TIME}", values[3]);
    html.replace("{DATE}", values[4]);
    
    html.replace("{DISPLAY_OPTIONS_TIME}", values[5]);
    html.replace("{DISPLAY_OPTIONS_WEATHER}", values[6]);
    html.replace("{DISPLAY_OPTIONS_DATE}", values[7]);

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
    html.replace("{USERPSW}", username);
    
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
    if(!ssid.isEmpty() && ! pass.isEmpty()){
        values[0] = ssid;
        values[1] = pass;
    }
	
	bool autoTime = server.hasArg("autoTime");
	if(!autoTime){
		String timeString = server.arg("time");
		int hour = timeString.substring(0,2).toInt(); 
		int min = timeString.substring(3,5).toInt(); 
		
		String dateString = server.arg("date");
		int year = dateString.substring(0,4).toInt();
		int month = dateString.substring(5,7).toInt();
		int day = dateString.substring(8,9).toInt();
	}
	
    changedValues = true;
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
