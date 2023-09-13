#include <Cryptography.h>

const char PSK[] = "Qu1cKBr0wNfoXjUm9pSOvERt4eLa5ydG";

uint32_t charIndex(char *s, char c)
{
    char *t = s;
    while ((*s != c) && (*s != '\0'))
        *s++;
    return (s - t);
}

void encrypt(char *s, char *d)
{
    char *t = d;
    uint32_t x;
    uint8_t POF = random(0, 15);
    uint8_t HN = 0;
    uint8_t LN = 0;

    *t++ = PSK[POF];

    for (x = 0; *s != '\0'; x++)
    {
        HN = (*s >> 4) & 0x0f;
        LN = (*s++ & 0x0f);
        *t++ = PSK[HN + POF];
        *t++ = PSK[LN + POF];
    }
}

void decrypt(char *s, char *d)
{
    char *t = d;
    uint8_t DOF = charIndex((char *)PSK, *s++);
    uint8_t HN = 0;
    uint8_t LN = 0;

    while (*s != '\0')
    {
        HN = (charIndex((char *)PSK, *s++) - DOF) << 4;
        LN = charIndex((char *)PSK, *s++) - DOF;
        *t++ = HN | LN;
    }
}