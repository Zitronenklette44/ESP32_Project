#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>

// Variables
#define ButtonTop 35
#define ButtonBottom 0

TFT_eSPI display = TFT_eSPI();

const char* ssid = "ESP_Test";
const char* pwd = "123456789";

WebServer server(80);

// put function declarations here:
void buttonCheck(void *parameter);
void wifiTask(void *parameter);

void handleRoot();
void handleSubmit();

void setup(){
	Serial.begin(115200);

	display.init();
	display.setRotation(1);

	display.fillScreen(TFT_BLACK);

	display.setTextColor(TFT_WHITE, TFT_BLACK);
	display.setTextSize(2);

	display.setCursor(20, 50);
	display.print("Hello World");

	pinMode(ButtonTop, INPUT_PULLUP);
	pinMode(ButtonBottom, INPUT_PULLUP);

	// Button Thread
	xTaskCreatePinnedToCore(buttonCheck, "Buttons", 4096, NULL, 1, NULL, 1);
	
	//Wifi Thread
	WiFi.softAP(ssid, pwd);
	xTaskCreatePinnedToCore(wifiTask, "wifi", 4096, NULL, 1, NULL, 1);
	server.on("/", handleRoot);
	server.on("/submit", handleSubmit);
	server.begin();
}

void loop(){
}

void buttonCheck(void *parameter){
	while (1){
		bool topPressed = digitalRead(ButtonTop) == 0;
		bool bottomPressed = digitalRead(ButtonBottom) == 0;

		if (topPressed){
			Serial.print("Top pressed");
		}

		if (bottomPressed){
			Serial.print("Bottom pressed");
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

void wifiTask(void* parameter){
	bool print = false;
	while(1){
		if(!print){
			Serial.println("Access Point started!");
			Serial.print("IP Address: ");
			Serial.println(WiFi.softAPIP());
			print = true;
		}
		server.handleClient();
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

void handleRoot() {
  server.send(200, "text/html",
    "<form action='/submit' method='get'>"
    "SSID: <input name='ssid'><br>"
    "Password: <input name='pass'><br>"
    "<input type='submit'>"
    "</form>");
}

void handleSubmit() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");
  server.send(200, "text/html", "Received! SSID=" + ssid + " Pass=" + pass);
  Serial.println("Received WiFi credentials:");
  Serial.println(ssid);
  Serial.println(pass);
}