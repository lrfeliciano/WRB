
#ifndef DEVICETIMER_h
#define DEVICETIMER_h
#include <Arduino.h>



void deviceTimerStart(TickType_t t, uint8_t reset);
void deviceTimerStop(void);
uint8_t deviceTimerState(void);

#endif