#pragma once
#include <Arduino.h>
#include "utils/Timestamp.h"
#include <utils/ConfigObserver.h>
#include "ConfigManager.h"

class Clock : public ConfigObserver{
private:
    
    static Clock* instance;
    bool autoTime;
    int gmtOffset;
    int daylightOffset;
    String path;
    bool started = false;


    Clock() : autoTime(true), gmtOffset(3600), daylightOffset(3600), path("pool.ntp.org"){};
public:
    ~Clock();

    void onConfigChange() override{
        autoTime = ConfigManager::getInstance()->getAutoTime();
        syncTimeNow();
    }

    static Clock* getInstance(){
        if(!instance){
            instance = new Clock();
            ConfigManager::getInstance()->addObserver(instance);
        }
        return instance;
    }

    void init();
    void setTime(Timestamp time);
    Timestamp getTime() const;
    void syncTimeNow();
    void setAutoTime(bool value);
    bool getAutoTime() const;
    bool isStarted() const;


};

// Clock::~Clock(){}


