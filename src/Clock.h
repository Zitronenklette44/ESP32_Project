#pragma once
#include <Arduino.h>
#include "utils/Timestamp.h"

class Clock
{
private:
    
    static Clock* instance;
    bool autoTime;
    int gmtOffset;
    int daylightOffset;
    String path;


    Clock() : autoTime(true), gmtOffset(3600), daylightOffset(3600), path("pool.ntp.org"){};
public:
    ~Clock();

    static Clock* getInstance(){
        if(!instance){
            instance = new Clock();
        }
        return instance;
    }

    void init();
    void setTime(Timestamp time);
    Timestamp getTime() const;
    void syncTimeNow();
    void setAutoTime(bool value);
    bool getAutoTime() const;


};

// Clock::~Clock(){}


