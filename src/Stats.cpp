#include <Arduino.h>
#include "Stats.h"

Stats* Stats::instance = nullptr;

Stats::~Stats(){}

uint64_t Stats::getUptime() const{
    return uptime;
}

void Stats::setUptime(uint64_t value){
    uptime = value;
}

void Stats::addUptime(uint64_t value){
    uptime += value;
}

bool Stats::getWifiStatus() const{
    return wifiStatus;
}

void Stats::setWifiStatus(bool value){
    wifiStatus = value;
}

String Stats::getIpAddress() const{
    return ipAddress;
}

void Stats::setIpAddress(String value){
    ipAddress = value;
}

Timestamp Stats::getLastConnection() const{
    return lastConnection;
}

void Stats::setLastConnection(Timestamp value){
    lastConnection = value;
}

Timestamp Stats::getLastApiCall() const{
    return lastApiCall;
}

void Stats::setLastApiCall(Timestamp value){
    lastApiCall = value;
}

void Stats::update(){
    unsigned long now = millis();
    // uptime
    uptime += now - lastUpdate;
    lastUpdate = now;
}