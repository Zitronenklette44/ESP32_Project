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
        return save();
    }

    File file = LittleFS.open(filePath, "r");
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

    apis.daily = doc["apis"]["daily"] | "";
    apis.hourly = doc["apis"]["hourly"] | "";

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
    login.adminOverridePassword= doc["login"]["adminOverridePassword"] | "";

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

void ConfigManager::setApiDaily(const String& v) { 
    apis.daily = v;
    updateObservers();
}
void ConfigManager::setApiHourly(const String& v) { 
    apis.hourly = v; 
    updateObservers();
}

// ---------------- WIFI ----------------
String ConfigManager::getWifiSSID() const { return wifi.ssid; }
String ConfigManager::getWifiPassword() const { return wifi.password; }

void ConfigManager::setWifiSSID(const String& v) { 
    wifi.ssid = v;
    updateObservers();
}
void ConfigManager::setWifiPassword(const String& v) { 
    wifi.password = v;
    updateObservers();
}

// ---------------- TIME ----------------
bool ConfigManager::getAutoTime() const { return time.autoTime; }
String ConfigManager::getTime() const { return time.time; }
String ConfigManager::getDate() const { return time.date; }

void ConfigManager::setAutoTime(bool v) { 
    time.autoTime = v; 
    updateObservers();
}
void ConfigManager::setTime(const String& v) { 
    time.time = v; 
    updateObservers();
}
void ConfigManager::setDate(const String& v) { 
    time.date = v; 
    updateObservers();
}

// --------------- DISPLAY ---------------
bool ConfigManager::getShowTime() const { return display.showTime; }
bool ConfigManager::getShowWeather() const { return display.showWeather; }
bool ConfigManager::getShowDate() const { return display.showDate; }

void ConfigManager::setShowTime(bool v) { 
    display.showTime = v; 
    updateObservers();
}
void ConfigManager::setShowWeather(bool v) { 
    display.showWeather = v; 
    updateObservers();
}
void ConfigManager::setShowDate(bool v) { 
    display.showDate = v; 
    updateObservers();
}

// --------------- LOGIN -----------------
String ConfigManager::getUserName() const { return login.userName; }
String ConfigManager::getUserPassword() const { return login.userPassword; }
String ConfigManager::getAdminOverrideName() const { return login.adminOverrideName; }
String ConfigManager::getAdminOverridePassword() const { return login.adminOverridePassword; }

void ConfigManager::setUserName(const String& v) { 
    login.userName = v; 
    updateObservers();
}
void ConfigManager::setUserPassword(const String& v) { 
    login.userPassword = v; 
    updateObservers();
}
void ConfigManager::setAdminOverrideName(const String& v) { 
    login.adminOverrideName = v; 
    updateObservers();
}
void ConfigManager::setAdminOverridePassword(const String& v) { 
    login.adminOverridePassword = v; 
    updateObservers();
}