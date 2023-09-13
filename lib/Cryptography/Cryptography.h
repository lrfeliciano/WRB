#ifndef CRYPTOGRAPHY_h
#define CRYPTOGRAPHY_h

#include <Arduino.h>


 
uint32_t charIndex(char *s, char c);

void encrypt(char *s, char *d);

void decrypt(char *s, char *d);
#endif