#pragma once
#include <Arduino.h>
#include <vector>
using std::vector;

class Logs{
private:
    int maxLogsize;
    vector<String> logEntrys;


    static Logs* instance;

    Logs() : maxLogsize(50){};
public:
    ~Logs();
    static Logs* getInstance(){
        if(!instance){
            instance = new Logs();
        }
        return instance;
    }

    void addLog(String entry);
    void setMaxLogSize(int maxSize);
    String getLogs();
    int getMaxLogSize();

};

