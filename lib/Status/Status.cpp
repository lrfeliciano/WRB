#include <Status.h>


Status::Status(uint8_t s):status_pin(s)
{
    pinMode(s,OUTPUT);
}

void Status::ledOn(void)
{
    digitalWrite(status_pin,HIGH);
}

void Status::ledOff(void)
{
    digitalWrite(status_pin,LOW);
}

void Status::ledToggle(void)
{
    digitalWrite(status_pin,!digitalRead(status_pin));
}

