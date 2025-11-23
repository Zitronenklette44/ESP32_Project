#pragma once

#include <Arduino.h>
#include <utils/Timestamp.h>

class Stats
{
private:
    uint32_t uptime;
    bool wifiStatus;
    String ipAddress;
    Timestamp lastConnection;
    Timestamp lastApiCall; 


public:
    Stats();
    ~Stats();

    uint32_t getUptime() const;
    void setUptime(uint32_t value);
    void addUptime(uint32_t value);

    bool getWifiStatus() const;
    void setWifiStatus(bool value);

    String getIpAddress() const;
    void setIpAddress(String value);

    Timestamp getLastConnection() const;
    void setLastConnection(Timestamp value);

    Timestamp getLastApiCall() const;
    void setLastApiCall(Timestamp value);

};
