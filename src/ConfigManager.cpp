#include "ConfigManager.h"

ConfigManager* ConfigManager::instance = nullptr;

bool ConfigManager::fileExists() {
    return LittleFS.exists(filePath);
}

bool ConfigManager::load() {
    // If file not found -> create with defaults
    if (!fileExists()) {
        Serial.println("Config file missing, creating default...");
        setDefaults();
        return false;//save();
    }

    File file = LittleFS.open(filePath, "r");
    Serial.println("---- CONFIG FILE RAW ----");
    while (file.available()) {
        Serial.write(file.read()); // raw output, no formatting
    }
    Serial.println("\n---- END ----");

    file = LittleFS.open(filePath, "r");
    if (!file) {
        Serial.println("Failed to open config file!");
        return false;
    }



    JsonDocument doc;

    DeserializationError err = deserializeJson(doc, file);
    file.close();

    if (err) {
        Serial.println("Invalid JSON, recreating default...");
        setDefaults();
        return save();
    }

    apis.daily = doc["apis"]["daily"] | "error";
    apis.hourly = doc["apis"]["hourly"] | "error";

    wifi.ssid = doc["wifiCredentials"]["ssid"] | "";
    wifi.password = doc["wifiCredentials"]["password"] | "";

    time.autoTime = doc["time"]["autoTime"] | false;
    time.time = doc["time"]["time"] | "";
    time.date = doc["time"]["date"] | "";

    display.showTime = doc["display"]["time"] | false;
    display.showWeather = doc["display"]["weather"] | false;
    display.showDate = doc["display"]["date"] | false;

    login.userName = doc["login"]["userName"] | "";
    login.userPassword = doc["login"]["userPassword"] | "";
    login.adminOverrideName = doc["login"]["adminOverrideName"] | "";
    login.adminOverridePassword = doc["login"]["adminOverridePassword"] | "";

    timeout.t1start = doc["timeout"]["t1start"] | "";
    timeout.t1end = doc["timeout"]["t1end"] | "";
    timeout.t2start = doc["timeout"]["t2start"] | "";
    timeout.t2end = doc["timeout"]["t2end"] | "";

    Serial.println("Successfully loaded configs");
    updateObservers();
    return true;
}

// Save config back to file
bool ConfigManager::save() {
    JsonDocument doc;

    doc["apis"]["daily"] = apis.daily;
    doc["apis"]["hourly"] = apis.hourly;

    doc["wifiCredentials"]["ssid"] = wifi.ssid;
    doc["wifiCredentials"]["password"] = wifi.password;

    doc["time"]["autoTime"] = time.autoTime;
    doc["time"]["time"] = time.time;
    doc["time"]["date"] = time.date;

    doc["display"]["time"] = display.showTime;
    doc["display"]["weather"] = display.showWeather;
    doc["display"]["date"] = display.showDate;

    doc["login"]["userName"] = login.userName;
    doc["login"]["userPassword"] = login.userPassword;
    
    doc["timeout"]["t1start"] = timeout.t1start;
    doc["timeout"]["t1end"] = timeout.t1end;
    doc["timeout"]["t2start"] = timeout.t2start;
    doc["timeout"]["t2end"] = timeout.t2end;

    File file = LittleFS.open(filePath, "w");
    if (!file) {
        Serial.println("Failed to open config file for write!");
        return false;
    }

    if (serializeJsonPretty(doc, file) == 0) {
        Serial.println("Failed to write JSON!");
        file.close();
        return false;
    }

    file.close();
    return true;
}

void ConfigManager::setDefaults() {
    Serial.println("reset Values");
    apis.daily  = "key";
    apis.hourly = "key";

    wifi.ssid     = "ssid";
    wifi.password = "pwd";

    time.autoTime = true;
    time.time     = "01:00";
    time.date     = "2020-01-01";

    display.showTime    = true;
    display.showWeather = true;
    display.showDate    = true;

    login.userName              = "admin";
    login.userPassword          = "";
    login.adminOverrideName     = "value";
    login.adminOverridePassword = "value";

    timeout.t1start = "22:00";
    timeout.t2end   = "06:00";
    timeout.t2start = "00:00";
    timeout.t2end   = "00:00";

}

void ConfigManager::addObserver(ConfigObserver* observer){
    observers.push_back(observer);
}

void ConfigManager::removeObserver(ConfigObserver* observer){
    observers.erase(
        std::remove(observers.begin(), observers.end(), observer),
        observers.end()
    );
}

void ConfigManager::updateObservers(){
    for(auto* obs : observers){
        obs->onConfigChange();
    }
}


// ---------------- API ----------------
String ConfigManager::getApiDaily() const { return apis.daily; }
String ConfigManager::getApiHourly() const { return apis.hourly; }

void ConfigManager::setApiDaily(const String& v, bool end) { 
    apis.daily = v;
    if(end) updateObservers();
}
void ConfigManager::setApiHourly(const String& v, bool end) { 
    apis.hourly = v; 
    if(end) updateObservers();
}

// ---------------- WIFI ----------------
String ConfigManager::getWifiSSID() const { return wifi.ssid; }
String ConfigManager::getWifiPassword() const { return wifi.password; }

void ConfigManager::setWifiSSID(const String& v, bool end) { 
    wifi.ssid = v;
    if(end) updateObservers();
}
void ConfigManager::setWifiPassword(const String& v, bool end) { 
    wifi.password = v;
    if(end) updateObservers();
}

// ---------------- TIME ----------------
bool ConfigManager::getAutoTime() const { return time.autoTime; }
String ConfigManager::getTime() const { return time.time; }
String ConfigManager::getDate() const { return time.date; }

void ConfigManager::setAutoTime(bool v, bool end) { 
    time.autoTime = v; 
    if(end) updateObservers();
}
void ConfigManager::setTime(const String& v, bool end) { 
    time.time = v; 
    if(end) updateObservers();
}
void ConfigManager::setDate(const String& v, bool end) { 
    time.date = v; 
    if(end) updateObservers();
}

// --------------- DISPLAY ---------------
bool ConfigManager::getShowTime() const { return display.showTime; }
bool ConfigManager::getShowWeather() const { return display.showWeather; }
bool ConfigManager::getShowDate() const { return display.showDate; }

void ConfigManager::setShowTime(bool v, bool end) { 
    display.showTime = v; 
    if(end) updateObservers();
}
void ConfigManager::setShowWeather(bool v, bool end) { 
    display.showWeather = v; 
    if(end) updateObservers();
}
void ConfigManager::setShowDate(bool v, bool end) { 
    display.showDate = v; 
    if(end) updateObservers();
}

// --------------- LOGIN -----------------
String ConfigManager::getUserName() const { return login.userName; }
String ConfigManager::getUserPassword() const { return login.userPassword; }
String ConfigManager::getAdminOverrideName() const { return login.adminOverrideName; }
String ConfigManager::getAdminOverridePassword() const { return login.adminOverridePassword; }

void ConfigManager::setUserName(const String& v, bool end) { 
    login.userName = v; 
    if(end) updateObservers();
}
void ConfigManager::setUserPassword(const String& v, bool end) { 
    login.userPassword = v; 
    if(end) updateObservers();
}
void ConfigManager::setAdminOverrideName(const String& v, bool end) { 
    login.adminOverrideName = v; 
    if(end) updateObservers();
}
void ConfigManager::setAdminOverridePassword(const String& v, bool end) { 
    login.adminOverridePassword = v; 
    if(end) updateObservers();
}

// --------------- Timeout -----------------
String ConfigManager::getTimeoutStart(int id) const{
    return id == 1 ? timeout.t1start : id == 2 ? timeout.t2start : "";
}
String ConfigManager::getTimeoutEnd(int id) const{
    return id == 1 ? timeout.t1end : id == 2 ? timeout.t2end : "";
}

void ConfigManager::setTimeoutStart(const String& v, int id, bool end){
    if(id == 1){
        timeout.t1start = v;
    }else if(id == 2){
        timeout.t2start = v;
    }
    if(end) updateObservers();
}
void ConfigManager::setTimeoutEnd(const String& v, int id, bool end){
    if(id == 1){
        timeout.t1end = v;
    }else if(id == 2){
        timeout.t2end = v;
    }
    if(end) updateObservers();
}