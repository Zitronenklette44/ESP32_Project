#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <Display.h>
#include "Wlan.h"
#include "Stats.h"
#include "Clock.h"
#include "Logs.h"
#include "Weather.h"

using namespace fs;


// Variables
#define ButtonTop 35
#define ButtonBottom 0

#define LOGSIZE 50

// classes
Display display;
Wlan wlan;

//Threads
TaskHandle_t buttonHandler = NULL;

// Functions
void buttonCheck(void *parameter);
void onWlanConnection(bool hourly);

void setup(){
	Logs::getInstance()->addLog("Started controller");
	Serial.begin(115200);

	if(!LittleFS.begin(true)){
        Serial.println("An error has occurred while mounting LittleFS");
	    Logs::getInstance()->addLog("Error while mounting LittleFS");
        return;
  	}

	LittleFS.mkdir("/images");

    ConfigManager::getInstance()->load();

    wlan.init();
    Clock::getInstance()->init();
    display.init();

	pinMode(ButtonTop, INPUT_PULLUP);
	pinMode(ButtonBottom, INPUT_PULLUP);
	
	// LittleFS.format();
}

bool trueSetup = false;
int lastHour = -1;
void loop(){
    bool showWeather = false;
	if(!trueSetup){
        Weather::getInstance();
        Clock::getInstance();
        Stats::getInstance;
        ConfigManager::getInstance()->updateObservers();
		// Button Thread
		Logs::getInstance()->addLog("started Button tracking");
		xTaskCreatePinnedToCore(buttonCheck, "Buttons", 4096, NULL, 1, &buttonHandler, 1);

        wlan.startWifi(5000);
        Clock::getInstance()->init();
        Weather::getInstance()->refreshData(false);
        Weather::getInstance()->refreshData(true);
        Serial.println("Setup Done");
		trueSetup = true;
	}

    Stats::getInstance()->update();
    display.update(showWeather);
    delay(100);

    if(showWeather){
        
    }


    String hourStr = Clock::getInstance()->getTime().trimTime().substring(0, 2);
    int currentHour = hourStr.toInt();

    if (currentHour != lastHour) {
        lastHour = currentHour;

        wlan.startWifi(5000);
        onWlanConnection(true);
    }
}

void buttonCheck(void *parameter){
    bool wasTopPressed = false;
    bool wasBottomPressed = false;
    const int debounceDelay = 50; // ms
    unsigned long lastTopChange = 0;
    unsigned long lastBottomChange = 0;

    while (1){
        bool topPressed = digitalRead(ButtonTop) == 0;
        bool bottomPressed = digitalRead(ButtonBottom) == 0;
        unsigned long now = millis();

        // Top Button
        if(topPressed != wasTopPressed && now - lastTopChange > debounceDelay){
            lastTopChange = now;
            if(topPressed && !wlan.isActive()){
                wlan.startWifi(15*60*1000);
                Serial.println("Wlan pressed");
                onWlanConnection(true);
            }
            wasTopPressed = topPressed;
        }

        // Bottom Button
        if(bottomPressed != wasBottomPressed && now - lastBottomChange > debounceDelay){
            lastBottomChange = now;
            if(bottomPressed){
                Serial.println("Closing WiFi");
                wlan.endWifi();
            }
            wasBottomPressed = bottomPressed;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void onWlanConnection(bool hourly){
    if(Stats::getInstance()->getWifiStatus()){
        Logs::getInstance()->addLog("Started Doing Stuff with Wlan");
        Clock::getInstance()->syncTimeNow();
        Weather::getInstance()->refreshData(hourly);
    }
}

