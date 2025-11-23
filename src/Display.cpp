#include <Display.h>

Display::Display() : tft(TFT_eSPI()), lastx(0), lasty(0){}
Display::~Display(){}

void Display::init(){
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
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

