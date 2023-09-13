#include <Relay.h>

Relay::Relay(void)
    : relay1Pin(RELAY_1_PIN), relay2Pin(RELAY_2_PIN), relay3Pin(RELAY_3_PIN), relay4Pin(RELAY_4_PIN)
{
    pinMode(relay1Pin, OUTPUT);
    pinMode(relay2Pin, OUTPUT);
    pinMode(relay3Pin, OUTPUT);
    pinMode(relay4Pin, OUTPUT);

#ifdef RELAY_DEBUG
    vTaskDelay(2000);
    Serial.begin(115200);
    Serial.println("Relay(void);");
#endif
}

Relay::Relay(uint8_t relay1, uint8_t relay2, uint8_t relay3, uint8_t relay4)
    : relay1Pin(relay1), relay2Pin(relay2), relay3Pin(relay3), relay4Pin(relay4)
{
    pinMode(relay1Pin, OUTPUT);
    pinMode(relay2Pin, OUTPUT);
    pinMode(relay3Pin, OUTPUT);
    pinMode(relay4Pin, OUTPUT);
#ifdef RELAY_DEBUG
    vTaskDelay(2000);
    Serial.begin(115200);
    Serial.println("Relay(uint8_t relay1, uint8_t relay2, uint8_t relay3, uint8_t relay4); ");
#endif
}

void Relay::begin(uint8_t rState1, uint8_t rState2, uint8_t rState3, uint8_t rState4,
                  uint8_t rDelay1, uint8_t rDelay2, uint8_t rDelay3, uint8_t rDelay4)
{
    relay1OnState = rState1;
    relay2OnState = rState2;
    relay3OnState = rState3;
    relay4OnState = rState4;
    relay1Delay = rDelay1 * 1000;
    relay2Delay = rDelay2 * 1000;
    relay3Delay = rDelay3 * 1000;
    relay4Delay = rDelay4 * 1000;
    digitalWrite(RELAY_1_PIN, !relay1OnState);
    digitalWrite(RELAY_2_PIN, !relay2OnState);
    digitalWrite(RELAY_3_PIN, !relay3OnState);
    digitalWrite(RELAY_4_PIN, !relay4OnState); 
}

void Relay::on(RelayType_t relayPin)
{
    switch (relayPin)
    {
    case 1:
        digitalWrite(relay1Pin, relay1OnState); // HIGH);
        break;
    case 2:
        digitalWrite(relay2Pin, relay2OnState); // HIGH);
        break;
    case 3:
        digitalWrite(relay3Pin, relay3OnState); // HIGH);
        break;
    case 4:
        digitalWrite(relay4Pin, relay4OnState); // HIGH);
        break;
    default:
        break;
    }

#ifdef RELAY_DEBUG
    Serial.print("Relay ");
    Serial.print(relayPin);
    Serial.println(" On.");
#endif
}

void Relay::off(RelayType_t relayPin)
{
    switch (relayPin)
    {
    case 1:
        digitalWrite(relay1Pin, !relay1OnState); //LOW);
        break;
    case 2:
        digitalWrite(relay2Pin, !relay2OnState); //LOW);
        break;
    case 3:
        digitalWrite(relay3Pin, !relay3OnState); //LOW);
        break;
    case 4:
        digitalWrite(relay4Pin, !relay4OnState); //LOW);
        break;
    default:
        break;
    }

#ifdef RELAY_DEBUG
    Serial.print("Relay ");
    Serial.print(relayPin);
    Serial.println(" Off.");
#endif
}

void Relay::toggle(RelayType_t relayPin)
{
    switch (relayPin)
    {
    case 1:
        digitalWrite(relay1Pin, !digitalRead(relay1Pin));
        break;
    case 2:
        digitalWrite(relay2Pin, !digitalRead(relay2Pin));
        break;
    case 3:
        digitalWrite(relay3Pin, !digitalRead(relay3Pin));
        break;
    case 4:
        digitalWrite(relay4Pin, !digitalRead(relay4Pin));
        break;
    default:
        break;
    }

#ifdef RELAY_DEBUG
    Serial.print("Relay ");
    Serial.print(relayPin);
    Serial.println(" Toggle.");
#endif
}

void Relay::pulse(RelayType_t relayPin)
{
    switch (relayPin)
    {
    case 1:
        digitalWrite(relay1Pin, HIGH);
        vTaskDelay(PULSE_DELAY);
        digitalWrite(relay1Pin, LOW);
        break;
    case 2:
        digitalWrite(relay2Pin, HIGH);
        vTaskDelay(PULSE_DELAY);
        digitalWrite(relay2Pin, LOW);
        break;
    case 3:
        digitalWrite(relay3Pin, HIGH);
        vTaskDelay(PULSE_DELAY);
        digitalWrite(relay3Pin, LOW);
        break;
    case 4:
        digitalWrite(relay4Pin, HIGH);
        vTaskDelay(PULSE_DELAY);
        digitalWrite(relay4Pin, LOW);
        break;
    default:
        break;
    }

#ifdef RELAY_DEBUG
    Serial.print("Relay ");
    Serial.print(relayPin);
    Serial.println(" On.");
#endif
}

void Relay::offDelay(RelayType_t relayPin)
{
    if (relayPin == RELAY_1)
    {
        // Serial.println("I'm here.");
        digitalWrite(relay1Pin, relay1OnState);
        if (relay1Delay)
        {
            vTaskDelay(relay1Delay);
            digitalWrite(relay1Pin, !relay1OnState);
        }
    }
    else if (relayPin == RELAY_2)
    {
        digitalWrite(relay2Pin, relay2OnState);
        if (relay2Delay)
        {
            vTaskDelay(relay2Delay);
            digitalWrite(relay2Pin, !relay2OnState);
        }
    }
    else if (relayPin == RELAY_3)
    {
        digitalWrite(relay3Pin, relay3OnState);
        if (relay3Delay)
        {
            vTaskDelay(relay3Delay);
            digitalWrite(relay3Pin, !relay3OnState);
        }
    }
    else if (relayPin == RELAY_4)
    {
        digitalWrite(relay4Pin, relay4OnState);
        if (relay4Delay)
        {
            vTaskDelay(relay4Delay);
            digitalWrite(relay4Pin, !relay4OnState);
        }
    }

#ifdef RELAY_DEBUG
    Serial.print("Relay ");
    Serial.print(relayPin);
    Serial.println(" On.");
#endif
}

void Relay::greenActivate(RelayType_t relayPin)
{
    // digitalWrite(relay1Pin, relay1OnState);
    if (relay1Delay)
    {
        vTaskDelay(relay1Delay);
        digitalWrite(relay1Pin, !relay1OnState);
    }

#ifdef RELAY_DEBUG
    Serial.print("Relay ");
    Serial.print(relayPin);
    Serial.println(" On.");
#endif
}

void Relay::orangeActivate(RelayType_t relayPin)
{
    // digitalWrite(relay2Pin, relay2OnState);
    if (relay2Delay)
    {
        vTaskDelay(relay2Delay);
        digitalWrite(relay2Pin, !relay2OnState);
    }

#ifdef RELAY_DEBUG
    Serial.print("Relay ");
    Serial.print(relayPin);
    Serial.println(" On.");
#endif
}

void Relay::redActivate(RelayType_t relayPin)
{
    // digitalWrite(relay3Pin, relay3OnState);
    if (relay3Delay)
    {
        vTaskDelay(relay3Delay);
        digitalWrite(relay3Pin, !relay3OnState);
    }

#ifdef RELAY_DEBUG
    Serial.print("Relay ");
    Serial.print(relayPin);
    Serial.println(" On.");
#endif
}

void Relay::greenOn(void)
{
    // Serial.println("\r\n\r\nvoid Relay::greenOn(void)");
    // Serial.print("relay1Pin: "); Serial.println(relay1Pin);
    // Serial.print("relay1OnState: "); Serial.println(relay1OnState);
    
    digitalWrite(relay1Pin, relay1OnState);
    
}

void Relay::greenOff(void)
{
    digitalWrite(relay1Pin, !relay1OnState);
}

void Relay::orangeOn(void)
{
    digitalWrite(relay2Pin, relay2OnState);
}

void Relay::orangeOff(void)
{
    digitalWrite(relay2Pin, !relay2OnState);
}

void Relay::redOn(void)
{
    digitalWrite(relay3Pin, relay3OnState);
}

void Relay::redOff(void)
{
    digitalWrite(relay3Pin, !relay3OnState);
}

void Relay::cancelOn(void)
{
    digitalWrite(relay4Pin, relay4OnState);
}

void Relay::cancelOff(void)
{
    digitalWrite(relay4Pin, !relay4OnState);
}

uint32_t Relay::greenOffDelay(void)
{
    return(relay1Delay);
}

uint32_t Relay::orangeOffDelay(void)
{
    return(relay2Delay);
}

uint32_t Relay::redOffDelay(void)
{
    return(relay3Delay);
}

uint32_t Relay::cancelOffDelay(void)
{
    return(relay4Delay);
}

uint8_t Relay::relayStateGet(void)
{
    return(relayState);
}

void Relay::relayStateSet(uint8_t rState)
{
    relayState |= (0x0f & rState);
}

void Relay::relayStateClear(uint8_t rState)
{
    relayState &= ~(0x0f & rState);
}