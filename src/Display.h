#pragma once
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <utils/DisplaySettings.h>
#include <utils/ConfigObserver.h>

class Display : public ConfigObserver{
private:
    TFT_eSPI tft;
    int lastx;
    int lasty;
    
    uint16_t gray;
    uint16_t blue;

    DisplaySettings displaySettings;

    static Display* instance;

public:
    Display();
    ~Display();

    static Display* getInstance(){
        if(!instance){
            instance = new Display();
        }
        return instance;
    }
    
    void onConfigChange() override{
        // uint8_t temp = digitalRead(TFT_BL);
        restartDisplay();
        // digitalWrite(TFT_BL, temp);
    }

    void init();
    void initColor();
    void clear();
    void print(int x, int y, String text);
    int getLastX() const;
    int getLastY() const;
    void setTextSize(int size);
    void setTextColor(uint32_t fg, uint32_t bg = TFT_BLACK);
    void update(bool showWeather);
    void drawDashboard();
    uint16_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    uint16_t mix(uint16_t c1, uint16_t c2, float t);
    DisplaySettings getDisplaySettings() const;
    void drawIcon(int id, int x, int y, uint16_t color);
    void restartDisplay();
    void displayOff();
    void displayOn();
    void fullRedraw();
};