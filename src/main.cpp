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
void checkTimeout();

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
    Weather::getInstance();
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
        Stats::getInstance();
        ConfigManager::getInstance()->updateObservers();
		// Button Thread
		Logs::getInstance()->addLog("started Button tracking");
		xTaskCreatePinnedToCore(buttonCheck, "Buttons", 4096, NULL, 1, &buttonHandler, 1);

        wlan.startWifi(5000);
        Clock::getInstance()->init();
        Weather::getInstance()->refreshData(false);
        Weather::getInstance()->refreshData(true);

        String hourStr = Clock::getInstance()->getTime().trimTime().substring(0, 2);
        lastHour = hourStr.toInt();

        Serial.println("Setup Done");
		trueSetup = true;
	}else{
        
        Stats::getInstance()->update();
        display.update(showWeather);
        delay(100);
        
        checkTimeout();

        String hourStr = Clock::getInstance()->getTime().trimTime().substring(0, 2);
        int currentHour = hourStr.toInt();
        
        if (currentHour != lastHour) {
            lastHour = currentHour;
            
            wlan.startWifi(5000);
            if(currentHour == 0){
                onWlanConnection(false);
            }else{
                onWlanConnection(true);
            }
        }

        if(wlan.restart){
            bool wlanState = Stats::getInstance()->getWifiStatus();
            if(!wlanState){
                wlan.startWifi(900000);
            }
        }
    }
    
}

bool display1off = false;
bool display2off = false;
void checkTimeout(){
    String time = Clock::getInstance()->getTime().trimTime();
    bool timeout1Valid = false;
    bool timeout2Valid = false;

    String timeout1start = ConfigManager::getInstance()->getTimeoutStart(1) + ":00";
    String timeout2start = ConfigManager::getInstance()->getTimeoutStart(2)+ ":00";
    String timeout1end = ConfigManager::getInstance()->getTimeoutEnd(1)+ ":00";
    String timeout2end = ConfigManager::getInstance()->getTimeoutEnd(2)+ ":00";

    timeout1Valid = !timeout1start.equals(timeout1end);
    timeout2Valid = !timeout2start.equals(timeout2end);

    // Serial.println("Timeout 1:" + timeout1start + " - " + timeout1end + " valid?->" + timeout1Valid);
    // Serial.println("Timeout 2:" + timeout2start + " - " + timeout2end + " valid?->" + timeout2Valid);
    // Serial.println("Time:" + time);
    bool change = false;

    if(timeout1Valid){
        if(time.equals(timeout1start)){
            Logs::getInstance()->addLog("timout 1 sleep");
            display1off = true;
            change = true;
        }else if(time.equals(timeout1end)){
            Logs::getInstance()->addLog("timeout 1 wakeup");
            display1off = false;
            change = true;
        }
    }
    if(timeout2Valid){
        if(time.equals(timeout2start)){
            Logs::getInstance()->addLog("timout 2 sleep");
            display2off = true;
            change = true;
        }else if(time.equals(timeout2end)){
            Logs::getInstance()->addLog("timeout 2 wakeup");
            display2off = false;
            change = true;
        }
    }

    if(!change) return;

    if(!display1off && !display2off){
        display.displayOn();
    }else if(display1off || display2off){
        display.displayOff();
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
                wlan.startWifi(900000);
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
        if(ConfigManager::getInstance()->getAutoTime()) Clock::getInstance()->syncTimeNow();
        Weather::getInstance()->refreshData(hourly);
    }
}

