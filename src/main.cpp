#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <PNGdec.h>
#include <Display.h>
#include "Wlan.h"

using namespace fs;


// Variables
#define ButtonTop 35
#define ButtonBottom 0

#define LOGSIZE 50

std::vector<String> values;
String logs[LOGSIZE];

// classes
Display displayM;
Wlan wlan(values);

//Threads
TaskHandle_t buttonHandler = NULL;

// Functions
void buttonCheck(void *parameter);
void setupValues();

void addLog(String s); 

void setup(){
	addLog("Started controller");
	Serial.begin(115200);
    setupValues();

	if(!LittleFS.begin(true)){
      Serial.println("An error has occurred while mounting LittleFS");
	  addLog("Error while mounting LittleFS");
      return;
  	}

	LittleFS.mkdir("/images");

    displayM.init();
    wlan.init(values[0], values[1]);

	pinMode(ButtonTop, INPUT_PULLUP);
	pinMode(ButtonBottom, INPUT_PULLUP);
	
	// LittleFS.format();
}

bool trueSetup = false;
void loop(){
	if(!trueSetup){
		// Button Thread
		addLog("started Button tracking");
		xTaskCreatePinnedToCore(buttonCheck, "Buttons", 4096, NULL, 1, &buttonHandler, 1);

		trueSetup = true;
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
                wlan.startWifi();
            }
            wasTopPressed = topPressed;
        }

        // Bottom Button
        if(bottomPressed != wasBottomPressed && now - lastBottomChange > debounceDelay){
            lastBottomChange = now;
            if(bottomPressed){
                Serial.println("Bottom pressed");
            }
            wasBottomPressed = bottomPressed;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

int currentLocation = 0;
void addLog(String s){
	if(currentLocation < LOGSIZE){
		logs[currentLocation] = s;
		currentLocation++;
	}else{
		for(int i = 0; i < LOGSIZE - 1; i++){
			logs[i] = logs[i + 1];
		}
		logs[LOGSIZE - 1] = s;
	}
}

void clearUploadFolder() {
    File dir = LittleFS.open("/images");
    File file = dir.openNextFile();

    while (file) {
        String path = file.name();
        Serial.println("Deleting: " + path);
        LittleFS.remove(path);
        file = dir.openNextFile();
    }
}

void setupValues(){
    values.push_back("ESP_Test");     // SSID
    values.push_back("123456789");    // SSID_PWD
    values.push_back("checked");     // autoTime checked = true
    values.push_back("");             // time HH:mm 
    values.push_back("");             // date YYYY-MM-dd
    values.push_back("checked");             // display_time
    values.push_back("checked");             // display_weather
    values.push_back("checked");             // display_date
}
