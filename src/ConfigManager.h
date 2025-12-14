#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>
#include <utils/ConfigObserver.h>

using std::vector;

// Simple struct holder for config categories
struct ApiConfig {
    String daily;
    String hourly;
};

struct WifiConfig {
    String ssid;
    String password;
};

struct TimeConfig {
    bool autoTime;
    String time;
    String date;
};

struct Timeout {
    String t1start;
    String t1end;
    String t2start;
    String t2end;
};

struct DisplayConfig {
    bool showTime;
    bool showWeather;
    bool showDate;
};

struct LoginConfig {
    String userName;
    String userPassword;
    String adminOverrideName;
    String adminOverridePassword;
};

class ConfigManager {
private:
    void setDefaults();
    bool fileExists();
    String filePath = "/coreValues.env";
    vector<ConfigObserver*> observers;
    static ConfigManager* instance;

    ConfigManager(){};

    ApiConfig apis;
    WifiConfig wifi;
    TimeConfig time;
    DisplayConfig display;
    LoginConfig login;
    Timeout timeout;
public:
    static ConfigManager* getInstance(){
        if(!instance){
            instance = new ConfigManager();
        }
        return instance;
    }

    bool load();
    bool save();
    void updateObservers();
    void addObserver(ConfigObserver* observer);
    void removeObserver(ConfigObserver* observer);

    // ---- API GET / SET ----
    String getApiDaily() const;
    String getApiHourly() const;
    void setApiDaily(const String& v, bool end = true);
    void setApiHourly(const String& v, bool end = true);

    // ---- WIFI GET / SET ----
    String getWifiSSID() const;
    String getWifiPassword() const;
    void setWifiSSID(const String& v, bool end = true);
    void setWifiPassword(const String& v, bool end = true);

    // ---- TIME GET / SET ----
    bool getAutoTime() const;
    String getTime() const;
    String getDate() const;
    void setAutoTime(bool v, bool end = true);
    void setTime(const String& v, bool end = true);
    void setDate(const String& v, bool end = true);
    
    // ---- DISPLAY GET / SET ----
    bool getShowTime() const;
    bool getShowWeather() const;
    bool getShowDate() const;
    void setShowTime(bool v, bool end = true);
    void setShowWeather(bool v, bool end = true);
    void setShowDate(bool v, bool end = true);
    
    // ---- LOGIN GET / SET ----
    String getUserName() const;
    String getUserPassword() const;
    String getAdminOverrideName() const;
    String getAdminOverridePassword() const;
    
    void setUserName(const String& v, bool end = true);
    void setUserPassword(const String& v, bool end = true);
    void setAdminOverrideName(const String& v, bool end = true);
    void setAdminOverridePassword(const String& v, bool end = true);

    // ---- Timeout GET / SET ----
    String getTimeoutStart(int id) const;
    String getTimeoutEnd(int id) const;
    
    void setTimeoutStart(const String& v, int id, bool end = true);
    void setTimeoutEnd(const String& v, int id, bool end = true);
    
};
