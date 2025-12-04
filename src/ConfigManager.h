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
    void setApiDaily(const String& v);
    void setApiHourly(const String& v);

    // ---- WIFI GET / SET ----
    String getWifiSSID() const;
    String getWifiPassword() const;
    void setWifiSSID(const String& v);
    void setWifiPassword(const String& v);

    // ---- TIME GET / SET ----
    bool getAutoTime() const;
    String getTime() const;
    String getDate() const;
    void setAutoTime(bool v);
    void setTime(const String& v);
    void setDate(const String& v);

    // ---- DISPLAY GET / SET ----
    bool getShowTime() const;
    bool getShowWeather() const;
    bool getShowDate() const;
    void setShowTime(bool v);
    void setShowWeather(bool v);
    void setShowDate(bool v);

    // ---- LOGIN GET / SET ----
    String getUserName() const;
    String getUserPassword() const;
    String getAdminOverrideName() const;
    String getAdminOverridePassword() const;

    void setUserName(const String& v);
    void setUserPassword(const String& v);
    void setAdminOverrideName(const String& v);
    void setAdminOverridePassword(const String& v);

};
