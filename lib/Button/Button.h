#ifndef BUTTON_h
#define BUTTON_h
#include <Arduino.h>

class Button
{
    public:
        Button(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);
        void    begin(void);

        void        ButtonFunction1Address(void (*f)());
        void        ButtonFunction1Process(void);
        uint8_t     Button1Read(void);

        void        ButtonFunction2Address(void (*f)());
        void        ButtonFunction2Process(void);
        uint8_t     Button2Read(void);
        uint8_t     Button2State(void);
        void        Button2InvertState(void);

        void        ButtonFunction3Address(void (*f)());
        void        ButtonFunction3Process(void);
        uint8_t     Button3Read(void);
        uint8_t     Button3State(void);
        void        Button3InvertState(void);

        void        ButtonFunction4Address(void (*f)());
        void        ButtonFunction4Process(void); 
        uint8_t     Button4Read(void);
        uint8_t     Button4State(void);
        void        Button4InvertState(void);
    private:
        void    (*butFunc1)(void);
        void    (*butFunc2)(void);
        void    (*butFunc3)(void);
        void    (*butFunc4)(void);
        uint8_t button_1_pin = 0;
        uint8_t button_2_pin = 0;
        uint8_t button_3_pin = 0;
        uint8_t button_4_pin = 0;

        uint8_t button_1_state = 0;
        uint8_t button_2_state = 1;
        uint8_t button_3_state = 0;
        uint8_t button_4_state = 0;
};
    
#endif