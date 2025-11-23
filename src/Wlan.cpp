#include <Wlan.h>

static Wlan* instance = nullptr;

String username = "admin";
String userPSW = "";
SessionToken sessionToken;

Wlan::Wlan(vector<String>& vals) : server(80), values(vals), wifiHandler(NULL), changedValues(false), startSSID(""), startPWD(""), active(false){
    instance = this; 
}

Wlan::~Wlan() {
    endWifi();
}

// Getter
bool Wlan::hasChangedValues() const {
    return changedValues;
}

bool Wlan::isActive() const{
    return active;
}



void Wlan::init(String ssid, String pwd) {
    startSSID = ssid;
    startPWD = pwd;
}

void Wlan::startWifi() {
    WiFi.softAP(startSSID, startPWD);   // start access point

    // register server routes
    server.on("/", [](){ instance->handleRoot(); });
    server.on("/index.html", [](){ instance->handleRoot(); });

    server.on("/login.html", [](){ instance->handleLogin(); });
    server.on("/loginConfirmation", HTTP_POST, [](){ 
        String recievedUserName = instance->server.arg("userName");
        String recievedUserPsw = instance->server.arg("userPSW");

        if(!recievedUserName.equals(username) || !recievedUserPsw.equals(userPSW)){
            instance->server.send(404, "text/plain", "Invalid Login info!");
            return;
        }

        sessionToken.newToken();
        String html = "<!DOCTYPE html><html><head>"
                    "<meta http-equiv='refresh' content='0; URL=/dev.html' />"
                    "</head><body></body></html>";

        instance->server.sendHeader("Set-Cookie", "session=" + sessionToken.getToken());
        instance->server.send(200, "text/html", html);  
    });
    
    server.on("/dev.html", [](){ instance->handleDev(); });
    
    server.on("/complete.html", HTTP_POST, [](){ instance->handleSubmit(); }, [](){ instance->handleFileUpload(); });
    server.on("/complete.html", HTTP_GET, [](){
        File f = LittleFS.open("/complete.html", "r");
        if(!f){
            instance->server.send(404, "text/plain", "File not found");
            return;
        }
        instance->server.streamFile(f, "text/html");
        f.close();
    });

    server.serveStatic("/style.css", LittleFS, "/style.css");
    server.serveStatic("/script.js", LittleFS, "/script.js");

    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.begin();
    active = true;

    // start task on Core 1
    xTaskCreatePinnedToCore(wifiTaskWrapper, "wifi", 4096, this, 1, &wifiHandler, 1);
}

void Wlan::endWifi() {
    if(wifiHandler != NULL) {
        vTaskDelete(wifiHandler);
        wifiHandler = NULL;
    }
    server.close();
    WiFi.softAPdisconnect(true);
    active = false;
}

void Wlan::wifiTaskWrapper(void* param){
    Wlan* self = (Wlan*)param;
    bool print = false;
	while(1){
		if(!print){
			Serial.println("Access Point started!");
			Serial.print("IP Address: ");
			Serial.println(WiFi.softAPIP());
			print = true;
		}
		self->server.handleClient();
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

void Wlan::handleRoot() {
    File file = LittleFS.open("/index.html", "r");
    if(!file){
        server.send(404, "text/plain", "File not found");
        return;
    }

    String html = "";
    while(file.available()){
        html += char(file.read());
    }

    html.replace("{SSID}", values[0]);
    html.replace("{SSIDPWD}", values[1]);

    html.replace("{AUOTTIME_SELECTED}", values[2]);
    html.replace("{TIME}", values[3]);
    html.replace("{DATE}", values[4]);
    
    html.replace("{DISPLAY_OPTIONS_TIME}", values[5]);
    html.replace("{DISPLAY_OPTIONS_WEATHER}", values[6]);
    html.replace("{DISPLAY_OPTIONS_DATE}", values[7]);

    //server.streamFile(file, "text/html");
    file.close();
    server.send(200, "text/html", html);
}

void Wlan::handleDev() {
    String cookie = server.header("Cookie");
    Serial.println("recieved cookie ->" +cookie);

    if(cookie.indexOf(/*"session=" + */sessionToken.getToken()) == -1){
        server.sendHeader("Location", "/login.html");
        server.send(302, "text/plain", "");
        Serial.println("invalid Session redirecting!");
        return;
    }

    File file = LittleFS.open("/dev.html", "r");
    if(!file){
        server.send(404, "text/plain", "File not found");
        return;
    }
    
    String html;
    while(file.available()){
        html += char(file.read());
    }
    //server.streamFile(file, "text/html");
    file.close();
    
    html.replace("", "");

    server.send(200, "text/html", html);


}

void Wlan::handleLogin() {
    String cookie = server.header("Cookie");

    if(cookie.indexOf("session=" + sessionToken.getToken()) != -1){
        server.sendHeader("Location", "/dev.html");
        server.send(302, "text/plain", "");
        return;
    }

    File file = LittleFS.open("/login.html", "r");
    if(!file){
        server.send(404, "text/plain", "File not found");
        return;
    }
    server.streamFile(file, "text/html");
    file.close();
}

void Wlan::handleSubmit() {
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
	
    changedValues = true;
	Serial.println("Received WiFi credentials:");
	Serial.println(ssid);
	Serial.println(pass);
}

void Wlan::handleFileUpload() {
    // handle uploaded file if needed
}

/*
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
}*/
