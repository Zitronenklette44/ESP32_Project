#include <Arduino.h>
#include <Clock.h>
#include "Stats.h"
#include <Logs.h>

Clock* Clock::instance = nullptr;

void Clock::init(){
    if(autoTime && Stats::getInstance()->getWifiStatus()){
        configTime(gmtOffset, daylightOffset, path.c_str());
        Logs::getInstance()->addLog("Connected Clock");
        started = true;
    } else Serial.println("Clock not in sync");
}

void Clock::setTime(Timestamp time){
    tm t = {};
    t.tm_year = time.year - 1900;
    t.tm_mon  = time.month - 1;
    t.tm_mday = time.day;
    t.tm_hour = time.hour;
    t.tm_min  = time.minute;
    t.tm_sec  = time.second;
    
    time_t epoch = mktime(&t);
    
    struct timeval now = { epoch, 0 };
    settimeofday(&now, nullptr);
    started = true;
}

Timestamp Clock::getTime() const{
    tm timeinfo;
    Timestamp t;
    if(getLocalTime(&timeinfo, 2000)){
        t.day = timeinfo.tm_mday;
        t.month = timeinfo.tm_mon + 1;
        t.year = timeinfo.tm_year + 1900;
        t.hour = timeinfo.tm_hour;
        t.minute = timeinfo.tm_min;
        t.second = timeinfo.tm_sec;
    }else{
        Logs::getInstance()->addLog("Failed to load Time!");
        t.day = 0;
        t.month = 0;
        t.year = 0;
        t.hour = 0;
        t.minute = 0;
        t.second = 0;
    }
    return t;
}

void Clock::syncTimeNow(){
    if(autoTime && Stats::getInstance()->getWifiStatus()){
        configTime(gmtOffset, daylightOffset, path.c_str());
        Logs::getInstance()->addLog("Connected Clock");
    }else Serial.println("Clock not in sync");
}

void Clock::setAutoTime(bool value){
    autoTime = value;   
}

bool Clock::getAutoTime() const{
    return autoTime;
}

Clock::~Clock(){}

bool Clock::isStarted() const{
    return started;
}
