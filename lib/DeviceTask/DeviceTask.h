#ifndef DEVICETASK_H
#define DEVICETASK_H
#include <Arduino.h>

#include <Button.h>
#include <Relay.h> 
#include <Status.h>
#include <DataProcess.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
 
#include <ESPmDNS.h>
#include <Update.h>
#include <index.h>
 
#include <DeviceTaskPriorities.h> 
#include <DeviceTimer.h>
/*NOTE: Retaining the 3.9 version as base firmware */
#define FW_VERSION  (float)3.9 
#define SHOW_CORE 
#define DEVICE_TYPE "WIFI"

/* 5 minutes */
#define DEVICE_HEART_BEAT 300000UL 

/* Relay Event Group */
#define RELAY_GREEN_EVENT   (uint32_t)0x01
#define RELAY_ORANGE_EVENT  (uint32_t)0x02
#define RELAY_RED_EVENT     (uint32_t)0x04
#define RELAY_CANCEL_EVENT  (uint32_t)0x08


void device_init_task(void);

#endif