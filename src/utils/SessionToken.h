#pragma once
#include <Arduino.h>
#include <Logs.h>

struct SessionToken {
    String token;
    int length = 30;
    long timestamp = 0;
    long maxAge = 0.25 * 60 * 60 * 1000; // in ms

    bool isExpired() const {
        return millis() - timestamp > maxAge;
    }

    String getToken() {
        return token;
    }

    String newToken() {
        String chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";
        token = "";
        for (int i = 0; i < length; i++) {
            token += chars.charAt(random(chars.length()));
        }
        timestamp = millis();
        Logs::getInstance()->addLog("generated New Token->" + token);
        return token;
    }
};
