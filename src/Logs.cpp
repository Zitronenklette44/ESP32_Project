#include <Logs.h>
#include "Clock.h"

Logs* Logs::instance = nullptr;

Logs::~Logs(){} 

void Logs::addLog(String entry){
    String time = "[";
    if(Clock::getInstance()->isStarted()) time +=  Clock::getInstance()->getTime().trimTime();
    else time += "NOT_STARTED";
    time += "] ";

    logEntrys.push_back(time + entry);
    if(logEntrys.size() > maxLogsize){
        logEntrys.erase(logEntrys.begin());
    }
    Serial.println("LOG-> " + time + entry);
}

void Logs::setMaxLogSize(int maxSize){
    maxLogsize = maxSize;
}

String Logs::getLogs(){
    String result = "";

    for(auto& e : logEntrys){
        result += e + "\n";
    }

    return result;
}

int Logs::getMaxLogSize(){
    return maxLogsize;
}