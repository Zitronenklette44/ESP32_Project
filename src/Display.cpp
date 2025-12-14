#include <Display.h>
#include "Clock.h"
#include "ConfigManager.h"
#include "Weather.h"
#include <Stats.h>

// #define TFT_BL 32

Display::Display() : tft(TFT_eSPI()), lastx(0), lasty(0) {}
Display::~Display() {}
bool firstTimeBoot = true;
int brightness = 128;

void Display::init(){
    // Serial.println("Init called...");
    if(firstTimeBoot){
        ConfigManager::getInstance()->addObserver(this);
        firstTimeBoot = false;

        pinMode(TFT_BL, OUTPUT);
    }
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.drawString("0", -100, -100);

    tft.setTextSize(6);
    tft.drawString("0", -100, -100);

    tft.setTextSize(2); // default back
    tft.drawString(" ", -100, -100);
    initColor();
    drawDashboard();
}

void Display::initColor(){
    gray = rgba(115, 113, 113, 1);
    blue = rgba(70, 173, 225, 1);
}

void Display::clear(){
    tft.fillScreen(TFT_BLACK);
    lastx = 0;
    lasty = 0;
}

void Display::print(int x, int y, String text){
    tft.drawString(text, x, y);
    lastx = x;
    lasty = y;
}

int Display::getLastX() const{
    return lastx;
}

int Display::getLastY() const{
    return lasty;
}

void Display::setTextSize(int size){
    tft.setTextSize(size);
}

void Display::setTextColor(uint32_t fg, uint32_t bg){
    tft.setTextColor(fg, bg);
}

String oldDate = "";
String oldTime = "";
bool oldWifi = false;
bool oldPrecipitation = false;
int updateCounter = 0;
void Display::update(bool showWeather){
    if (showWeather){

        return;
    }

    int yPos = 0;

    String date = Clock::getInstance()->getTime().trimDate();
    if (!date.equals(oldDate) && ConfigManager::getInstance()->getShowDate()){
        yPos += 10;
        tft.setTextSize(2);
        tft.drawString(" ", -100, -100);
        int width = tft.textWidth(date);
        tft.fillRect((240 - width) / 2, yPos, width, 15, gray);
        tft.drawString(date, (240 - width) / 2, yPos);
        oldDate = date;
        yPos = 52;
    }

    String time = Clock::getInstance()->getTime().trimTime();
    if (!time.equals(oldTime) && ConfigManager::getInstance()->getShowTime()){
        tft.setTextSize(6);
        tft.drawString(" ", -100, -100);
        if (yPos != 52){
            yPos = 67 - tft.fontHeight() / 2;
        }
        int width = tft.textWidth(time.substring(0, 5));
        if (!time.substring(0, 5).equals(oldTime.substring(0, 5))){
            tft.fillRect((240 - width) / 2 - 5, yPos, width, tft.fontHeight() + 3, TFT_BLACK);
            tft.drawString(time.substring(0, 5), (240 - width) / 2 - 5, yPos);
        }
        tft.setTextSize(2);
        tft.drawString(" ", -100, -100);
        tft.fillRect((240 - width) / 2 + width - 3, yPos, width, tft.fontHeight() + 7, TFT_BLACK);
        tft.drawString(time.substring(6), (240 - width) / 2 + width - 3, yPos);
        oldTime = time;
    }
    
    bool wifi = Stats::getInstance()->getWifiStatus();
    if(wifi != oldWifi){
        if(!wifi) tft.fillRect(10, 115, 20, 20, TFT_BLACK); 
        else drawIcon(0, 10, 115, TFT_CYAN);
        oldWifi = wifi;
    }

    bool weather = Weather::getInstance()->hasUpdate();
    if((weather || updateCounter % 100 == 0) && ConfigManager::getInstance()->getShowWeather()){
        tft.setTextSize(2);
        
        if(updateCounter < 300){
            float maxTemp = Weather::getInstance()->getMaxTemperatur();
            tft.fillRect(40, 110, 100, 25, TFT_BLACK);
            drawIcon(1, 40, 110, TFT_GREENYELLOW);
            tft.drawString(String(maxTemp), 65, 115);
        }else if(updateCounter < 700){
            float temp = Weather::getInstance()->getTemperatur();
            tft.fillRect(40, 110, 100, 25, TFT_BLACK);
            drawIcon(3, 40, 110, TFT_LIGHTGREY);
            tft.drawString(String(temp), 65, 115);
        }else {
            float minTemp = Weather::getInstance()->getMinTemperature();
            tft.fillRect(40, 110, 100, 25, TFT_BLACK);
            drawIcon(2, 40, 110, TFT_RED);
            tft.drawString(String(minTemp), 65, 115);
        }

        bool precipitation = Weather::getInstance()->getPrecipitation() != 0;
        if(oldPrecipitation != precipitation){
            if(precipitation){
                tft.fillRect(150, 110, 30, 20, TFT_BLACK);
                drawIcon(5, 150, 110, gray);
            }else{
                tft.fillRect(150, 110, 30, 20, TFT_BLACK);
                // drawIcon(4, 150, 110, TFT_YELLOW);
            }
        }

    }

    if(updateCounter >= 1000) updateCounter = 0;
    else updateCounter++;
}

void Display::drawDashboard(){
    if (ConfigManager::getInstance()->getShowDate()){
        tft.fillRoundRect(0, 0, 240, 30, 15, gray);
        tft.fillRect(0, 0, 240, 15, gray);

        String date = Clock::getInstance()->getTime().trimDate();
        int width = tft.textWidth(date);
        tft.drawString(date, (240 - width) / 2, 10);
    }

    // drawIcon(0, 10, 100, TFT_CYAN);
    // drawIcon(1, 50, 100, TFT_GREENYELLOW);
    // drawIcon(2, 110, 100, TFT_RED);
    // drawIcon(3, 160, 100, TFT_YELLOW);
    // drawIcon(4, 210, 100, gray);
}

// converts 0-255 RGB to 16-bit color
// alpha value only for vs-Code dosn't have any effects
uint16_t Display::rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    return ((r & 0xF8) << 8) | // 5 bits red
           ((g & 0xFC) << 3) | // 6 bits green
           (b >> 3);           // 5 bits blue
}

// blends two colors by t (0.0 - 1.0)
uint16_t Display::mix(uint16_t c1, uint16_t c2, float t){
    uint8_t r1 = (c1 >> 11) & 0x1F;
    uint8_t g1 = (c1 >> 5) & 0x3F;
    uint8_t b1 = c1 & 0x1F;

    uint8_t r2 = (c2 >> 11) & 0x1F;
    uint8_t g2 = (c2 >> 5) & 0x3F;
    uint8_t b2 = c2 & 0x1F;

    uint8_t r = r1 + (r2 - r1) * t;
    uint8_t g = g1 + (g2 - g1) * t;
    uint8_t b = b1 + (b2 - b1) * t;

    return (r << 11) | (g << 5) | b;
}

void Display::drawIcon(int id, int x, int y, uint16_t color){
    switch (id){

    // 0 = Wifi icon
    case 0:{
        tft.drawArc(x + 10, y + 10, 10, 8, 110, 250, color, TFT_BLACK);
        tft.drawArc(x + 10, y + 10, 7, 5, 110, 250, color, TFT_BLACK);
        tft.drawArc(x + 10, y + 10, 4, 2, 110, 250, color, TFT_BLACK);
        tft.fillCircle(x + 10, y + 10, 2, color);
    }
    break;

    // 1 = Arrow up
    case 1:{
        tft.fillTriangle(
            x, y + 20,
            x + 20, y + 20,
            x + 10, y,
            color);
    }
    break;

    // 2 = Arrow down
    case 2:{
        tft.fillTriangle(
            x, y,
            x + 20, y,
            x + 10, y + 20,
            color);
    }
    break;
    // 3 = Arrow right
    case 3:{
        tft.fillTriangle(
            x, y,
            x, y + 20,
            x + 20, y + 10,
            color);
    }
    break;

        // 4 = Sun icon
    case 4:{
        // center circle
        tft.fillCircle(x + 12, y + 12, 8, color); // simple sun core

        // rays

        // vertical
        tft.drawLine(x + 12, y + 0, x + 12, y + 4, color);
        tft.drawLine(x + 12, y + 20, x + 12, y + 16, color);

        // horizontal
        tft.drawLine(x + 0, y + 12, x + 4, y + 12, color);
        tft.drawLine(x + 20, y + 12, x + 16, y + 12, color);

        // diagonal top-left
        tft.drawLine(x + 4, y + 4, x + 7, y + 7, color);
        // diagonal top-right
        tft.drawLine(x + 20 - 4, y + 4, x + 20 - 7, y + 7, color);
        // diagonal bottom-left
        tft.drawLine(x + 4, y + 20 - 4, x + 7, y + 20 - 7, color);
        // diagonal bottom-right
        tft.drawLine(x + 20 - 4, y + 20 - 4, x + 20 - 7, y + 20 - 7, color);
    }
    break;

        // 5 = Rain icon
    case 5:{
        // cloud base
        tft.fillCircle(x + 7, y + 10, 6, color);
        tft.fillCircle(x + 14, y + 10, 6, color);
        tft.fillRect(x + 5, y + 10, 14, 8, color);

        // raindrops 

        // left drop
        tft.fillTriangle(
            x + 6, y + 20,
            x + 4, y + 24,
            x + 8, y + 24,
            blue);

        // right drop
        tft.fillTriangle(
            x + 18, y + 20,
            x + 16, y + 24,
            x + 20, y + 24,
            blue);

        // center drop (lower)
        tft.fillTriangle(
            x + 12, y + 22,
            x + 10, y + 26,
            x + 14, y + 26,
            blue);
    }
    break;

    default:
        // nothing
        break;
    }
}

void Display::restartDisplay(){
    oldDate = "";
    oldTime = "";
    oldWifi = false;
    init();
}

void Display::displayOff(){
    digitalWrite(TFT_BL, LOW); 
}

void Display::displayOn(){
    digitalWrite(TFT_BL, HIGH);
}