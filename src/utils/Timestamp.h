#pragma once

#include <Arduino.h>

struct Timestamp {
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    String toString() const {
        char buffer[25];
        sprintf(buffer, "%02u.%02u.%04u %02u:%02u:%02u",
                day, month, year,
                hour, minute,second);
        return String(buffer);
    }
};