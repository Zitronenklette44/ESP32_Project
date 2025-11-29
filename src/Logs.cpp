#include <Logs.h>
#include "Clock.h"

Logs* Logs::instance = nullptr;

Logs::~Logs(){} 

void Logs::addLog(String entry, bool newLine){
    if(!newLine){
        if(!logEntrys.empty()){
            logEntrys.back() += entry; 
            Serial.println("LOG-> " + logEntrys.back());
        }
    return;
    }
    String time = "[";
    if(Clock::getInstance()->isStarted()) time +=  Clock::getInstance()->getTime().trimTime();
    else time += "NOT_STARTED";
    time += "] ";

    logEntrys.push_back(time + entry);
    if(logEntrys.size() > maxLogsize){
        logEntrys.pop_front();
    }
    Serial.println("LOG-> " + time + entry);
}

void Logs::addLog(String entry){
    addLog(entry, true);
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