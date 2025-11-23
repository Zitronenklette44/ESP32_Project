#include <Arduino.h>
#include "Stats.h"

Stats::Stats() : uptime(0), wifiStatus(false), ipAddress(""), lastConnection{1,1,1970,0,0,0}, lastApiCall{1,1,1970,0,0,0} {}

Stats::~Stats(){}

uint32_t Stats::getUptime() const{
    return uptime;
}

void Stats::setUptime(uint32_t value){
    uptime = value;
}

void Stats::addUptime(uint32_t value){
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