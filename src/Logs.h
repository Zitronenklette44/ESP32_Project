#pragma once
#include <Arduino.h>
#include <vector>
#include <deque>
using std::deque;

class Logs{
private:
    int maxLogsize;
    deque<String> logEntrys;


    static Logs* instance;

    Logs() : maxLogsize(100){};
public:
    ~Logs();
    static Logs* getInstance(){
        if(!instance){
            instance = new Logs();
        }
        return instance;
    }

    void addLog(String entry);
    void addLog(String entry, bool newLine);
    void setMaxLogSize(int maxSize);
    String getLogs();
    int getMaxLogSize();

};

