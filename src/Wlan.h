#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <vector>
using std::vector;

class Wlan {
private:
    WebServer server;            // the webserver
    vector<String>& values;      // reference to shared values
    TaskHandle_t wifiHandler;    // FreeRTOS task handle
    bool changedValues;          // track if values changed
    String startSSID;
    String startPWD;
    bool active;

    // internal helpers
    void handleRoot();
    void handleDev();
    void handleSubmit();
    void handleFileUpload();

    // wrapper task for FreeRTOS
    static void wifiTaskWrapper(void* param);

public:
    Wlan(vector<String>& values);
    ~Wlan();

    void init(String ssid, String pwd);
    void startWifi();
    void endWifi();

    bool hasChangedValues() const;
    bool isActive() const;
};
