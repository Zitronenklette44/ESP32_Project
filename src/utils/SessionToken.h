#pragma once
#include <Arduino.h>

struct SessionToken{
    String token;
    int length = 30;
    bool wasSet = false;

    String getToken() {
        String chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";
        if(wasSet){
            return token;
        }else{
            token = "";
            for(int i = 0; i < length; i++){
                token += chars.charAt(random(0, chars.length()));
            }
            wasSet = true;
            return token;
        }
    };

    String newToken(){
        wasSet = false;
        return getToken();
    };


};