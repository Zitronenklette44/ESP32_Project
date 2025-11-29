#pragma once
#include <Arduino.h>



class Weather{
private:
    String api = "https://api.open-meteo.com/v1/forecast?latitude=48.2285&longitude=8.7923&daily=temperature_2m_max,temperature_2m_min,precipitation_probability_max&hourly=temperature_2m,precipitation,precipitation_probability&current=temperature_2m,precipitation&timezone=auto&forecast_days=3";
    static Weather* instance;

    float temp;
    float precipitation;
    float maxTemp;
    float minTemp;
    float maxPrecipitation;

    Weather();

public:
    ~Weather();

    static Weather* getInstance(){
        if(!instance){
            instance = new Weather();
        }
        return instance;
    }

    void refreshData();
    float getTemperatur() const;
    float getPrecipitation() const;
    float getMaxTemperatur() const;
    float getMinTemperature() const;
    float getMaxPrecipitation() const;

};
