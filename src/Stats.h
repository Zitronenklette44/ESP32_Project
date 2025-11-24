#pragma once

#include <Arduino.h>
#include <utils/Timestamp.h>

class Stats
{
private:
    uint64_t uptime;
    unsigned long lastUpdate;
    bool wifiStatus;
    String ipAddress;
    Timestamp lastConnection;
    Timestamp lastApiCall; 
    static Stats* instance;

    Stats() : uptime(0), lastUpdate(0), wifiStatus(false), ipAddress(""), lastConnection{1,1,1970,0,0,0}, lastApiCall{1,1,1970,0,0,0} {}

public:
    ~Stats();

    static Stats* getInstance(){
        if(!instance){
            instance = new Stats();
        }
        return instance;
    };

    uint64_t getUptime() const;
    void setUptime(uint64_t value);
    void addUptime(uint64_t value);

    bool getWifiStatus() const;
    void setWifiStatus(bool value);

    String getIpAddress() const;
    void setIpAddress(String value);

    Timestamp getLastConnection() const;
    void setLastConnection(Timestamp value);

    Timestamp getLastApiCall() const;
    void setLastApiCall(Timestamp value);

    void update();

};
