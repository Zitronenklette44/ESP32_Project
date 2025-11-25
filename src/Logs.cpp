#include <Logs.h>
#include "Clock.h"

Logs* Logs::instance = nullptr;

Logs::~Logs(){} 

void Logs::addLog(String entry){
    logEntrys.push_back("[" + Clock::getInstance()->getTime().trimTime() + "] " + entry);
    if(logEntrys.size() > maxLogsize){
        logEntrys.erase(logEntrys.begin());
    }
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