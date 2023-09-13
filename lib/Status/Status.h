
#ifndef STATUS_h
#define STATUS_h
#include <Arduino.h>

class Status
{
    public:
        Status(uint8_t s);
        void ledOn(void);
        void ledOff(void);
        void ledToggle(void); 
    private: 
        uint8_t status_pin = 0; 
};

#endif