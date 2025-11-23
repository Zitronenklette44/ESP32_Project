#pragma once
#include <TFT_eSPI.h>
#include <Arduino.h>

class Display{
private:
    TFT_eSPI tft;
    int lastx;
    int lasty;

public:
    Display();
    ~Display();

    void init();
    void clear();
    void print(int x, int y, String text);
    int getLastX() const;
    int getLastY() const;
    void setTextSize(int size);
};