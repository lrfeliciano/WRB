#ifndef DEVICEDATABASE_h
#define DEVICEDATABASE_h

#include <Arduino.h>
#include "ArduinoJson.h"
#define ENCRYPTED_DB
 
#define DEVICEDATABASE_ADDR "/config.wrb" 

uint8_t deviceDatabaseInit(void);

void deviceDatabaseOpen(void);

void deviceDatabaseSave(void);

void deviceDatabaseRemove(void);

void deviceDatabasePrint(void);

void deviceDatabaseCreate(void);

#endif