#include <Arduino.h>
#include <Clock.h>
#include "Stats.h"
#include <Logs.h>

Clock* Clock::instance = nullptr;

void Clock::init(){
    syncTimeNow();
}

void Clock::setTime(Timestamp time){
    // Serial.println("Time->" + time.toString());
    tm t = {};
    t.tm_year = time.year - 1900;
    t.tm_mon  = time.month - 1;
    t.tm_mday = time.day;
    t.tm_hour = time.hour;
    t.tm_min  = time.minute;
    t.tm_sec  = time.second;

    // Serial.println("newTime->" + String(t.tm_year) + "."+ String(t.tm_mon) + "."+ String(t.tm_mday) + " "+ String(t.tm_hour) + ":"+ String(t.tm_min) + ":"+ String(t.tm_sec));
    
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
        started = true;
    }else {
        if(!autoTime){
            Timestamp t;
            String time = ConfigManager::getInstance()->getTime();
            t.hour = time.substring(0, 2).toInt();
            t.minute = time.substring(3, 5).toInt();
            t.second = 0;
            
            String date = ConfigManager::getInstance()->getDate();
            t.year = date.substring(0, 4).toInt();
            t.month = date.substring(5, 7).toInt();
            t.day = date.substring(8, 10).toInt();
            setTime(t);
        }else{
            Serial.println("Clock not in sync");
            started = false;
        }
    }
}

void Clock::setAutoTime(bool value){
    ConfigManager::getInstance()->setAutoTime(value);   
}

bool Clock::getAutoTime() const{
    return ConfigManager::getInstance()->getAutoTime();
}

Clock::~Clock(){}

bool Clock::isStarted() const{
    return started;
}
