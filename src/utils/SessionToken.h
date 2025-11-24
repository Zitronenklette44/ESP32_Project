#pragma once
#include <Arduino.h>

struct SessionToken {
    String token;
    int length = 30;
    long timestamp = 0;
    long maxAge = 0.25 * 60 * 60 * 1000; // in ms

    bool isExpired() const {
        return millis() - timestamp > maxAge;
    }

    String getToken() {
        if (!token.isEmpty()) {
            return token; // still valid
        }

        if(isExpired()){
            token = "EXPIRED";
            return token;
        }

        // generate new token
        String chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";
        token = "";

        for (int i = 0; i < length; i++) {
            token += chars.charAt(random(chars.length()));
        }

        timestamp = millis();
        return token;
    }

    String newToken() {
        token = "";
        return getToken();
    }
};
