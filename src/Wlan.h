#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <vector>
#include <utils/SessionToken.h>
#include <utils/ConfigObserver.h>
#include "ConfigManager.h"

using std::vector;


class Wlan : public ConfigObserver{
private:
    WebServer server;            // the webserver
    TaskHandle_t wifiHandler;    // FreeRTOS task handle
    String startSSID;
    String startPWD;
    bool active;
    bool stopTask = false;
    long ttl;
    long currentTtlMs = 0; 

    String username = "admin";
    String userPSW = "";
    String adminName = "admin";
    String adminPSW = "";

    // internal helpers
    void handleRoot();
    void handleDev();
    void handleLogin();
    void handleSubmit();
    void handleFileUpload();
    bool validateToken(String cookieHeader);

    // wrapper task for FreeRTOS
    static void wifiTaskWrapper(void* param);

public:
    void onConfigChange() override{
        startSSID = ConfigManager::getInstance()->getWifiSSID();
        startPWD = ConfigManager::getInstance()->getWifiPassword();
        username = ConfigManager::getInstance()->getUserName();
        userPSW = ConfigManager::getInstance()->getUserPassword();
        adminName = ConfigManager::getInstance()->getAdminOverrideName();
        adminPSW = ConfigManager::getInstance()->getAdminOverridePassword();
    }
    bool restart;

    Wlan();
    ~Wlan();

    void init();
    bool startWifi(long ttlMs);
    void endWifi();

    bool isActive() const;
};
