#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <PNGdec.h>

using namespace fs;


// Variables
#define ButtonTop 35
#define ButtonBottom 0

#define LOGSIZE 50

TFT_eSPI display = TFT_eSPI();

String ssid = "ESP_Test";
String pwd = "123456789";
String logs[LOGSIZE];

WebServer server(80);

File uploadFile;

//Threads
TaskHandle_t buttonHandler = NULL;
TaskHandle_t wifiHandler = NULL;

// Functions
void buttonCheck(void *parameter);
void wifiTask(void *parameter);
void startWifi();

void handleRoot();
void handleSubmit();
void handleDev();
void handleFileUpload();

void showPNGFile(String path);

void addLog(String s); 

void setup(){
	addLog("Started controller");
	Serial.begin(115200);

	if(!LittleFS.begin(true)){
      Serial.println("An error has occurred while mounting LittleFS");
	  addLog("Error while mounting LittleFS");
      return;
  	}

	LittleFS.mkdir("/images");

	display.init();
	display.setRotation(1);

	display.fillScreen(TFT_BLACK);

	display.setTextColor(TFT_WHITE, TFT_BLACK);
	display.setTextSize(2);

	display.setCursor(20, 50);
	display.print("Hello World");

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

        File f = LittleFS.open("/images/fruehling.png", "r");
        if(!f) Serial.println("File not found!");
        else Serial.println("File exists!");
        f.close();

		showPNGFile("/images/fruehling.png");

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
            if(topPressed){
                startWifi();
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
    File file = LittleFS.open("/index.html", "r");
    if(!file){
        server.send(404, "text/plain", "File not found");
        return;
    }
    server.streamFile(file, "text/html");
    file.close();
}

void handleDev() {
    File file = LittleFS.open("/dev.html", "r");
    if(!file){
        server.send(404, "text/plain", "File not found");
        return;
    }
    server.streamFile(file, "text/html");
    file.close();
}

void handleSubmit() {
	File file = LittleFS.open("/complete.html", "r");
    if(file){
        server.streamFile(file, "text/html");
        file.close();
    }

	String ssid = server.arg("ssid");
	String pass = server.arg("ssidPwd");
	
	bool autoTime = server.hasArg("autoTime");
	if(!autoTime){
		String timeString = server.arg("time");
		int hour = timeString.substring(0,2).toInt(); 
		int min = timeString.substring(3,5).toInt(); 
		
		String dateString = server.arg("date");
		int year = dateString.substring(0,4).toInt();
		int month = dateString.substring(5,7).toInt();
		int day = dateString.substring(8,9).toInt();
	}
	
	//server.send(200, "text/html", "Received! SSID=" + ssid + " Pass=" + pass);
	Serial.println("Received WiFi credentials:");
	Serial.println(ssid);
	Serial.println(pass);
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

void startWifi(){
	//Wifi Thread
	addLog("started Wifi");
	WiFi.softAP(ssid, pwd);

	server.on("/", handleRoot);
	server.on("/index.html", handleRoot);
    server.on("/dev.html", handleDev);
    server.on("/complete.html", HTTP_POST, handleSubmit, handleFileUpload);
    server.on("/complete.html", HTTP_GET, [](){
    File f = LittleFS.open("/complete.html", "r");
        if(!f){
            server.send(404, "text/plain", "File not found");
            return;
        }
        server.streamFile(f, "text/html");
        f.close();
    });


    server.serveStatic("/style.css", LittleFS, "/style.css");
    server.serveStatic("/script.js", LittleFS, "/script.js");
	xTaskCreatePinnedToCore(wifiTask, "wifi", 4096, NULL, 1, &wifiHandler, 1);
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.begin();
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

String path = "";
void handleFileUpload() {
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        Serial.println("Upload start: " + upload.filename);

		String inputName = upload.name;
			
		int dotIndex = upload.filename.lastIndexOf(".");
		String extension = upload.filename.substring(dotIndex);

		path = "/images/" + inputName + extension;
        
		Serial.println("Saving to: " + path);

        File f = LittleFS.open(path, "w");
        f.close();
    }

    if (upload.status == UPLOAD_FILE_WRITE) {
        File f = LittleFS.open(path, "a");
        f.write(upload.buf, upload.currentSize);
        f.close();
    }

    if (upload.status == UPLOAD_FILE_END) {
        Serial.println("Upload finished");
    }
}


// for displaying an png file currently not working do to unknown issues with opening the file
PNG png;
File pngFile;

// open callback
void* pngOpen(const char *filename, int *pFileSize) {
    File* pf = new File(); 
    
    *pf = LittleFS.open(filename, "r"); 
    
    if (!(*pf)) { 
        Serial.println("PNG file cannot be opened");
        delete pf; 
        return nullptr;
    }

    if (pFileSize) *pFileSize = pf->size();
    
    return pf; 
}



void pngClose(void *pFile) {
    File *f = (File*)pFile;
    if(f){
        f->close();
        delete f;
    }
}


// read callback
int pngRead(PNGFILE *pFile, uint8_t *pBuf, int iLen) {
    File *f = (File*)pFile;
    return f->read(pBuf, iLen);
}

// seek callback
int pngSeek(PNGFILE *pFile, int32_t iPosition) {
    File *f = (File*)pFile;
    return f->seek(iPosition) ? 0 : -1;
}

int pngDraw(PNGDRAW *pDraw) {
    // pDraw->iWidth pixels in RGB888
    uint16_t line[pDraw->iWidth];
    for(int x=0; x<pDraw->iWidth; x++){
        uint8_t r = pDraw->pPixels[x*3 + 0];
        uint8_t g = pDraw->pPixels[x*3 + 1];
        uint8_t b = pDraw->pPixels[x*3 + 2];
        line[x] = ((r & 0xF8)<<8) | ((g & 0xFC)<<3) | (b>>3);
    }
    display.pushImage(0, pDraw->y, pDraw->iWidth, 1, line); // x = 0
    return 1;
}

void showPNGFile(String path) {
    int16_t rc = png.open(path.c_str(), pngOpen, pngClose, pngRead, pngSeek, pngDraw);

    if(rc != PNG_SUCCESS){
        Serial.print("Failed to open PNG. Error code: ");
        Serial.println(rc);
        return; // Abbruch
    }
    
    display.startWrite();
    png.decode(NULL, 0);
    png.close();
    display.endWrite();
    Serial.println("PNG drawn successfully");
}
