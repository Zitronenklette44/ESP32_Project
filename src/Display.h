#pragma once
#include <TFT_eSPI.h>
#include <Arduino.h>

class Display{
private:
    TFT_eSPI tft;
    int lastx;
    int lasty;
    
    uint16_t gray;
    uint16_t asd;

public:
    Display();
    ~Display();
    
    void init();
    void initColor();
    void clear();
    void print(int x, int y, String text);
    int getLastX() const;
    int getLastY() const;
    void setTextSize(int size);
    void setTextColor(uint32_t fg, uint32_t bg = TFT_BLACK);
    void update();
    void drawDashboard();
    uint16_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    uint16_t mix(uint16_t c1, uint16_t c2, float t);
};