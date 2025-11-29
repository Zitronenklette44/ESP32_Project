#include <Arduino.h>
#include <Weather.h>
#include <Stats.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Clock.h"

Weather* Weather::instance = nullptr;

Weather::Weather(): temp(0), precipitation(0), maxTemp(0), minTemp(0), maxPrecipitation(0) {}

Weather::~Weather(){}

void Weather::refreshData(){
    if(Stats::getInstance()->getWifiStatus()){
        HTTPClient http;
        http.begin(api);

        int code = http.GET();
        if(code == 200){
            Stats::getInstance()->setLastApiCall(Clock::getInstance()->getTime());
            String body = http.getString();
            // Serial.println("========WeatherData==========");
            // Serial.print(body);
            // Serial.println("=============================");


            JsonDocument doc;
            deserializeJson(doc, body);

            temp = doc["current"]["temperature_2m"];
            precipitation = doc["current"]["precipitation"];
            maxTemp = doc["daily"]["temperature_2m_max"];
            minTemp = doc["daily"]["temperature_2m_min"];
            maxPrecipitation = doc["daily"]["precipitation_probability_max"];
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
