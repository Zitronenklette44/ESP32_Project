#include <Arduino.h>
#include <Weather.h>
#include <Stats.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Clock.h"

Weather* Weather::instance = nullptr;

Weather::Weather(): temp(0), precipitation(0), maxTemp(0), minTemp(0), maxPrecipitation(0) {}

Weather::~Weather(){}

void Weather::refreshData(bool hourly){
    if(Stats::getInstance()->getWifiStatus()){
        Serial.println("hourly->" + api_hourly);
        Serial.println("daily->" + api_daily);
        HTTPClient http;
        if(hourly){
            http.begin(api_hourly);
        }else{
            http.begin(api_daily);
        }

        int code = http.GET();
        if(code == 200){
            update = true;
            Stats::getInstance()->setLastApiCall(Clock::getInstance()->getTime());
            String body = http.getString();
            // Serial.println("========WeatherData==========");
            // Serial.print(body);
            // Serial.println("=============================");


            JsonDocument doc;
            deserializeJson(doc, body);

            if(hourly){
                temp = doc["current"]["temperature_2m"];
                precipitation = doc["current"]["precipitation"];
            }else{
                maxTemp = doc["daily"]["temperature_2m_max"][0] | 0.0;
                minTemp = doc["daily"]["temperature_2m_min"][0] | 0.0;
                maxPrecipitation = doc["daily"]["precipitation_probability_max"][0] | 0.0;
            }
            
        }
    }
}

float Weather::getTemperatur() const{
    return temp;
}

float Weather::getPrecipitation() const{
    return precipitation;
}

float Weather::getMaxTemperatur() const{
    return maxTemp;
}

float Weather::getMinTemperature() const{
    return minTemp;
}

float Weather::getMaxPrecipitation() const{
    return maxPrecipitation;
}

bool Weather::hasUpdate() {
    if(update){
        update = false;
        return true;
    }
    return update;
}