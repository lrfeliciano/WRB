#ifndef RELAY_h
#define RELAY_h
#include <Arduino.h>

//#define RELAY_DEBUG       
#define RELAY_STATE_GREEN   1
#define RELAY_STATE_ORANGE  2
#define RELAY_STATE_RED     4
#define RELAY_STATE_CANCEL  8

#define RELAY_1_PIN         23
#define RELAY_2_PIN         22
#define RELAY_3_PIN         21
#define RELAY_4_PIN         19

#define PULSE_DELAY         1000
typedef enum
{
    RELAY_1 = 1,
    RELAY_2,
    RELAY_3,
    RELAY_4,
    RELAY_GREEN = 1,
    RELAY_ORANGE,
    RELAY_RED,
    RELAY_CANCEL,
    RELAY_NONE = 0
}RelayType_t;

class Relay
{
    public:
        Relay(void);
        Relay(uint8_t relay1, uint8_t relay2, uint8_t relay3, uint8_t relay4); 
        void begin(uint8_t rState1, uint8_t rState2, uint8_t rState3, uint8_t rState4, 
                    uint8_t rDelay1, uint8_t rDelay2, uint8_t rDelay3,uint8_t rDelay4);

        void on(RelayType_t relayPin);
        void off(RelayType_t relayPin);
        void toggle(RelayType_t relayPin);
        void pulse(RelayType_t relayPin);  
        void offDelay(RelayType_t relayPin); 
        void greenActivate(RelayType_t relayPin);
        void orangeActivate(RelayType_t relayPin);
        void redActivate(RelayType_t relayPin);
        void cancelActivate(RelayType_t relayPin);
        
        void greenOn(void);
        void greenOff(void);

        void orangeOn(void);
        void orangeOff(void);

        void redOn(void);
        void redOff(void);

        void cancelOn(void);
        void cancelOff(void);

        uint32_t greenOffDelay(void);
        uint32_t orangeOffDelay(void);
        uint32_t redOffDelay(void);
        uint32_t cancelOffDelay(void);

        uint8_t relayStateGet(void);
        void    relayStateSet(uint8_t rState);
        void    relayStateClear(uint8_t rState);

    private:
        uint8_t relayState;
        uint8_t relay1Pin;
        uint8_t relay2Pin;
        uint8_t relay3Pin;
        uint8_t relay4Pin;
        uint8_t relay1OnState;
        uint8_t relay2OnState;
        uint8_t relay3OnState;
        uint8_t relay4OnState;
        uint32_t relay1Delay;
        uint32_t relay2Delay;
        uint32_t relay3Delay;
        uint32_t relay4Delay;

};

#endif