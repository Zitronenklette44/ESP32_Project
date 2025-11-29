#include <Display.h>

#define black TFT_BLACK

Display::Display() : tft(TFT_eSPI()), lastx(0), lasty(0){}
Display::~Display(){}

void Display::init(){
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    initColor();
    drawDashboard();
}

void Display::initColor(){
    gray = rgba(115, 113, 113, 1);
    asd = rgba(146, 47, 47, 1);
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

void Display::update(){

}

void Display::drawDashboard(){
    tft.drawRoundRect(0, 0, 237, 30, 15, gray);
}

// converts 0-255 RGB to 16-bit color
// alpha value only for vs-Code dosn't have any effects
uint16_t Display::rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    return ((r & 0xF8) << 8) |   // 5 bits red
           ((g & 0xFC) << 3) |   // 6 bits green
           (b >> 3);             // 5 bits blue
}

// blends two colors by t (0.0 - 1.0)
uint16_t Display::mix(uint16_t c1, uint16_t c2, float t){
    uint8_t r1 = (c1 >> 11) & 0x1F;
    uint8_t g1 = (c1 >> 5)  & 0x3F;
    uint8_t b1 =  c1        & 0x1F;

    uint8_t r2 = (c2 >> 11) & 0x1F;
    uint8_t g2 = (c2 >> 5)  & 0x3F;
    uint8_t b2 =  c2        & 0x1F;

    uint8_t r = r1 + (r2 - r1) * t;
    uint8_t g = g1 + (g2 - g1) * t;
    uint8_t b = b1 + (b2 - b1) * t;

    return (r << 11) | (g << 5) | b;
}
