#pragma once
#include <Arduino.h>
#include <utils/ConfigObserver.h>
#include "ConfigManager.h"


class Weather : public ConfigObserver{
private:
    String api_daily;
    String api_hourly;
    static Weather* instance;

    float temp;
    float precipitation;
    float maxTemp;
    float minTemp;
    float maxPrecipitation;

    bool update = false;

    Weather();

public:
    ~Weather();

    void onConfigChange() override{
        api_daily = ConfigManager::getInstance()->getApiDaily();
        api_hourly = ConfigManager::getInstance()->getApiHourly();
    }

    static Weather* getInstance(){
        if(!instance){
            instance = new Weather();
            ConfigManager::getInstance()->addObserver(instance);
        }
        return instance;
    }

    void refreshData(bool hourly);
    float getTemperatur() const;
    float getPrecipitation() const;
    float getMaxTemperatur() const;
    float getMinTemperature() const;
    float getMaxPrecipitation() const;

    bool hasUpdate();

};
