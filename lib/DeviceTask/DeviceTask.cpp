#include <DeviceTask.h>

#include <Cryptography.h>
#include <ESPAsyncWebServer.h>
#include <freertos/FreeRTOSConfig.h>

#include <HTTPClient.h>
#include <HTTPUpdate.h> 


/* GREEN RELAY BUTTON */
#define DEVICE_BUTTON_1 34 
/* ORANGE RELAY BUTTON */
#define DEVICE_BUTTON_2 35  
/* CANCEL RELAY BUTTON */
#define DEVICE_BUTTON_3 33
/* RED RELAY BUTTON */
#define DEVICE_BUTTON_4 32

#define DEVICE_STATUS_LED   18

#define DEVICE_OTA_MQTT_MSG_LENGTH       1024

Relay relay(RELAY_1_PIN, RELAY_2_PIN, RELAY_3_PIN, RELAY_4_PIN);
Button button(DEVICE_BUTTON_1, DEVICE_BUTTON_2, DEVICE_BUTTON_3, DEVICE_BUTTON_4);
Status status(DEVICE_STATUS_LED);


char dataSegment[COMMAND_SET_MAX][49];

WiFiClient wrbWifi;
PubSubClient mqtt(wrbWifi);

DeviceType_t device;
 
/* Task Handle Declaration */
TaskHandle_t deviceWifiConnectionTaskHandle;
TaskHandle_t deviceMqttConnectionTaskHandle;
TaskHandle_t deviceConfigurationTaskHandle;
TaskHandle_t deviceButtonTaskHandle; 
TaskHandle_t deviceOTATaskHandle; 

/* Relay Task Handle */
TaskHandle_t deviceRelayDataProcessTaskHandle;
TaskHandle_t deviceRelayDelayOffTaskHandle;
TaskHandle_t deviceRelayGreenTaskHandle;
TaskHandle_t deviceRelayOrangeTaskHandle;
TaskHandle_t deviceRelayRedTaskHandle;
TaskHandle_t deviceRelayCancelTaskHandle;

TaskHandle_t statusBinkTaskHandle;  

/* Queue Handle Declaration */
xQueueHandle relayDelayOffQueueHandle = NULL;
xQueueHandle relayGreenQueueHandle = NULL;
xQueueHandle relayOrangeQueueHandle = NULL;
xQueueHandle relayRedQueueHandle = NULL;
xQueueHandle relayCancelQueueHandle = NULL;
xQueueHandle configurationDataSegmentQueueHandle = NULL; 
xQueueHandle deviceOTAQueueHandle = NULL; 

/* Semaphore Handle Mutex Declaration */
xSemaphoreHandle relayDelayOffMutex = 0;
xSemaphoreHandle relayGreenMutex = 0;
xSemaphoreHandle relayOrangeMutex = 0;
xSemaphoreHandle relayRedMutex = 0;
xSemaphoreHandle relayCancelMutex = 0;
xSemaphoreHandle configurationMutex = 0;

/* Semaphore Handle Binary Declaration */
xSemaphoreHandle relayReleaseMutex = 0;
xSemaphoreHandle relayGreenOffMutex = 0;
xSemaphoreHandle relayOrangeOffMutex = 0;
xSemaphoreHandle relayRedOffMutex = 0;
xSemaphoreHandle relayCancelOffMutex = 0;

/* Event Group Handle */
EventGroupHandle_t relayEvent; 


 

#define WIFI_MULTI 

void status_blink_task(void *p)
{  
    status.ledOff();
    vTaskDelay(100);
    status.ledOn(); 
    vTaskDelete(NULL);
}

void device_mqtt_callback(char *topic, byte *payload, unsigned int length)
{
    char CID[10];
    char LID[10];
    char alertType[10];
    char groupCode[10];
    
    unsigned int posIndex = 0;
    unsigned int index = 0; 

    Serial.println("\r\n\r\n---------------MQTT RECEIVE-------------------");
    Serial.printf("[MQTT CB] Topic: %s\r\n", topic);

    /* Wait for the hearbeat event and check if delay publish is not active */
    if((strcmp(topic,device.mqtt.event.hbMSQ.topic) == 0x00) && (device.hb.active == 0x00))
    {
        /* Store the time for the hearbeat to publish */
        device.hb.duration = device.hb.time + millis();
        /* Activate heartbeart delay function. */
        device.hb.active = 0x01;
        Serial.printf("[MQTT CB] HB Activated. Time: %u.\r\n",device.hb.time); 
    }

    /* Check if the topic is from command/alert */
    if(strcmp(topic,device.mqtt.commands.topic) == 0x00) 
    {
        RelayType_t rel;
        Serial.print("[MQTT CB] Commands Message: "); 

        /* Filter out the commands/alert information */
        for (int i = 0; i < length; i++, posIndex++)
        { 
            Serial.print((char)payload[i]); 

            if (payload[i] == ',')
            {
                i++;
                index++;
                posIndex = 0;
            }

            if (index == 0)
            {
                CID[posIndex] = payload[i];
                CID[posIndex+1] = '\0';
            }
            else if (index == 1)
            {
                LID[posIndex] = payload[i];
                LID[posIndex+1] = '\0';
            }
            else if (index == 2)
            {
                alertType[posIndex] = payload[i];
                alertType[posIndex+1] = '\0';
            }
            else if (index == 3)
            {
                groupCode[posIndex] = payload[i];
                groupCode[posIndex+1] = '\0'; 
            }
        }

        Serial.printf("\r\n[MQTT CB] CID: %s | LID: %s | AT: %s | GC: %s\r\n",CID,LID,alertType,groupCode); 

        /* Check Relay Channel and Group */
        if (strcmp(alertType, "C") == 0x00)  
        {
            if (strchr(device.relay.cancel.group, (int)groupCode[0]) != 0)
            {
                /* assigning alerts */
                rel = RELAY_4;
                Serial.println("[MQTT CB] Cancel Semaphore give."); 
            } 
        }
        /* NOTE: Swapping group and semaphore condition due to conflict once the alert recieve have different group */
        else if ((strcmp(alertType, "G") == 0x00) && (strchr(device.relay.green.group, (int)groupCode[0]) != 0)) 
        {
            /* take mutex */
            if (xSemaphoreTake(relayGreenMutex, 1)) 
            {
                /* assigning alerts */
                rel = RELAY_GREEN;
                Serial.println("[MQTT CB] Green Semaphore give."); 
                /* release mutex */
                xSemaphoreGive(relayGreenMutex);
            } 
        }
        
        else if ((!strcmp(alertType, "O")) && (strchr(device.relay.orange.group, (int)groupCode[0]) != 0))  
        {
            /* take mutex */
            if (xSemaphoreTake(relayOrangeMutex, 1))  
            {
                /* assigning alerts */
                rel = RELAY_ORANGE;
                Serial.println("[MQTT CB] Orange Semaphore give."); 
                /* release mutex */
                xSemaphoreGive(relayOrangeMutex);
            } 
        }
        else if (!strcmp(alertType, "R") && (strchr(device.relay.red.group, (int)groupCode[0]) != 0))  
        { 
            /* take mutex */
            if (xSemaphoreTake(relayRedMutex, 1))  
            {
                /* assigning alerts */
                rel = RELAY_RED;
                Serial.println("[MQTT CB] Red Semaphore give."); 
                /* release mutex */
                xSemaphoreGive(relayRedMutex);
            } 
        }

        /* Check to make sure that there is alert before sending queue */
        if (rel > RELAY_NONE)
        { 
            Serial.printf("[MQTT CB] Command %u Queued.\r\n",rel); 
            /* Sending queue to activate off sequence if available for specific alert */
            xQueueSend(relayDelayOffQueueHandle, &rel, 500); 
        }
    } 
    /* Check if the topic is from configration */
    else if (strcmp(topic, device.mqtt.configuration.topic) == 0x00)
    {
        Serial.print("[MQTT CB] Configuration Message:"); 
        for (int i = 0; i < length; i++, posIndex++)
        { 
            Serial.print((char)payload[i]); 
        }
        Serial.println();
        /* making sure datasegment is clear */
        memset(dataSegment, '\0', sizeof(dataSegment));
        /* process data */
        dataProcess((char *)payload, dataSegment); 
        /* Release the configuration mutex for configuration start */
        xSemaphoreGive(configurationMutex);
    }
    /* Check if the topic is from firmware update */
    else if (!strcmp(topic,device.mqtt.event.firmwareUpdateByID.topic) || !strcmp(topic,device.mqtt.event.firmwareUpdateByCID.topic) || !strcmp(topic,device.mqtt.event.firmwareUpdateByLID.topic))
    {
        strncpy(device.mqtt.event.firmwareUpdateByID.payload,(char *)payload,length);
        /* Puting the termination so that no excess data is included */
        device.mqtt.event.firmwareUpdateByID.payload[length] = '\0';
        Serial.printf("[MQTT CB] OTA FW Messages: %s\r\n",device.mqtt.event.firmwareUpdateByID.payload); 
        /* Send the URL for the OTA */
        xQueueSend(deviceOTAQueueHandle,device.mqtt.event.firmwareUpdateByID.payload,1000);
    } 
}
 
/* NOTE: Added function to Get the hearbeat delay value convert it from char to unsigned int */
uint8_t device_hb_delay(char *s)
{ 
    uint8_t hbDelay = 0;
    /* Convertion of the last 2 char of MAC into integer */ 
    hbDelay = (*s > '9' ? (*(s++) - 55) : (*(s++) - 48)) << 4;
    hbDelay |= (*s > '9' ? (*s - 55) : (*s - 48));
    return hbDelay;
}


void device_check_database(void)
{
    Serial.println("----------------DEVICE CHECK DATABASE---------------"); 
    String MAC; 
    
    /* Make sure device is clear */
    memset(&device, '\0', sizeof(device));

    /* Get WiFi MAC */
    device.MAC = WiFi.macAddress();
    MAC = device.MAC;

    /* Removing the colon */
    device.MAC.replace(":", ""); 

    /* NOTE: added to get the heart beat time */
    /* getting the last 2 hex of the MAC and convert it to heartbeat delay */
    device.hb.time = device_hb_delay((char*)device.MAC.substring(10,12).c_str()) * 1000; 
    Serial.println("\r\nDevice Details:");   

    /* Convert MAC to lower case  */
    device.MAC.toLowerCase();
    Serial.printf("MAC: %s MAC Low key: %s\r\nHB Time: %u\r\n",MAC.c_str(),device.MAC,device.hb.time);  

    /* Check if the database is newly created */
    if(deviceDatabaseInit() & 0x02)
    {
        /* Device ID designation */
        sprintf(device.ID.watch, "b000%s", device.MAC.c_str()); 
        Serial.print("New Watch ID: ");
        Serial.println(device.ID.watch);
        /* Store it on the database. */
        deviceDatabaseSave();
    }
    
    /* Get database and store it on the device*/
    getDeviceInformation(&device);

    /* Make sure that the device ID is not null */
    if(strcmp(device.ID.watch,"") == 0x00)
    {
        sprintf(device.ID.watch, "b000%s", device.MAC.c_str()); 
        Serial.print("New Watch ID: ");
        Serial.println(device.ID.watch);
        /* Store all the information */
        setDeviceInformation(&device);
    } 
    Serial.println("----------------------------------------------\r\n");  
}
  

const long buttonInterval = 500;

void device_button1_function(void)
{
    uint8_t pressCounter = 0;
    unsigned long prevTime = millis(); 

    /* Check if the button is still activated */
    while (button.Button1Read())
    {
        /* get current time */
        unsigned long curTime = millis();
        /* check if assigned interval is equal or greater */
        if ((curTime - prevTime) >= buttonInterval)
        {
            /* preserve current time for referenct to the next interval */
            prevTime = curTime;
            /* Press counter increament */
            pressCounter++; 

            Serial.printf("[BTN 1] Pressed %u.\r\n",pressCounter);
            /* start status blink task to notify the user that the button is pressed */
            xTaskCreatePinnedToCore(status_blink_task, "StatusBlink", 1024, NULL, DEVICE_TASKPRIORITIES_LED_STATUS, &statusBinkTaskHandle,PRO_CPU_NUM);

        }
        /* once it reach the press counter threshold then activate mqtt publish alert */
        if(pressCounter >=  10)
        { 
            /* Publish alert */
            mqtt.publish(device.mqtt.event.cancel.topic,device.mqtt.event.cancel.payload);
            Serial.printf("[BTN 1] Publish Cancel Alert.\r\n");
            break;
        }
    } 

    pressCounter = 0;
}

/* NOTE: Changes, remove the cancel publish */
void device_button2_function(void)
{
    uint8_t pressCounter = 0;
    unsigned long prevTime = millis(); 

    while (button.Button2Read() == button.Button2State())
    {
        /* get current time */
        unsigned long curTime = millis();
        /* check if assigned interval is equal or greater */
        if ((curTime - prevTime) >= buttonInterval)
        {
            /* preserve current time for referenct to the next interval */
            prevTime = curTime;
            /* Press counter increament */
            pressCounter++; 

            Serial.printf("[BTN 2] Pressed %u.\r\n",pressCounter);
            /* start status blink task to notify the user that the button is pressed */
            xTaskCreatePinnedToCore(status_blink_task, "StatusBlink", 1024, NULL, DEVICE_TASKPRIORITIES_LED_STATUS, &statusBinkTaskHandle,PRO_CPU_NUM);
        }

        /* once it reach the press counter threshold then activate mqtt publish alert */
        if(pressCounter >=  10)
        { 
            if(!button.Button2State())
            {
                /* Publish CANCEL alert */
                mqtt.publish(device.mqtt.event.cancel.topic,device.mqtt.event.cancel.payload);
            }
            else
            {
                /* Publish RED alert */
                mqtt.publish(device.mqtt.event.red.topic,device.mqtt.event.red.payload);
            }
            Serial.printf("[BTN 2] Publish %s Alert.\r\n",button.Button2State() ? "Red" : "Cancel"); 
            button.Button2InvertState();
            break;
        }
    } 
    pressCounter = 0;
}

/* NOTE: Changes, remove the cancel publish */
void device_button3_function(void)
{
    uint8_t pressCounter = 0;
    unsigned long prevTime = millis(); 
 
    while (button.Button3Read() == button.Button3State())
    {
        /* get current time */
        unsigned long curTime = millis();
        /* check if assigned interval is equal or greater */
        if ((curTime - prevTime) >= buttonInterval)
        {
            /* preserve current time for referenct to the next interval */
            prevTime = curTime;
            /* Press counter increament */
            pressCounter++; 

            Serial.printf("[BTN 3] Pressed %u.\r\n",pressCounter);
            /* start status blink task to notify the user that the button is pressed */
            xTaskCreatePinnedToCore(status_blink_task, "StatusBlink", 1024, NULL, DEVICE_TASKPRIORITIES_LED_STATUS, &statusBinkTaskHandle,PRO_CPU_NUM);
        }

        /* once it reach the press counter threshold then activate mqtt publish alert */
        if(pressCounter >=  10)
        { 
            if(button.Button3State())
            {
                /* Publish CANCEL alert */
                mqtt.publish(device.mqtt.event.cancel.topic,device.mqtt.event.cancel.payload);
            }
            else
            {
                /* Publish RED alert */
                mqtt.publish(device.mqtt.event.red.topic,device.mqtt.event.red.payload);
            }
            Serial.printf("[BTN 3] Publish %s Alert.\r\n", button.Button3State() ? "Cancel" : "Red"); 
            button.Button3InvertState();
            break;
        }
    } 
    pressCounter = 0;
}

void device_button4_function(void)
{
    uint8_t pressCounter = 0;
    unsigned long prevTime = millis(); 
    
    /* Check if the button is still activated */
    while (!button.Button4Read())
    {
        /* get current time */
        unsigned long curTime = millis();
        /* check if assigned interval is equal or greater */
        if ((curTime - prevTime) >= buttonInterval)
        {
            /* preserve current time for referenct to the next interval */
            prevTime = curTime;
            /* Press counter increament */
            pressCounter++; 

            Serial.printf("[BTN 4] Pressed %u.\r\n",pressCounter);
            /* start status blink task to notify the user that the button is pressed */
            xTaskCreatePinnedToCore(status_blink_task, "StatusBlink", 1024, NULL, DEVICE_TASKPRIORITIES_LED_STATUS, &statusBinkTaskHandle,PRO_CPU_NUM);
        }

        /* once it reach the press counter threshold then activate mqtt publish alert */
        if(pressCounter >=  10)
        { 
            /* Publish alert */
            mqtt.publish(device.mqtt.event.cancel.topic,device.mqtt.event.cancel.payload);
            Serial.printf("[BTN 4] Publish Cancel Alert.\r\n");
            break;
        }
    }  
    pressCounter = 0;
}

void device_button_task(void *p)
{ 
    Serial.printf("\r\n[Button Task] Device Button Task Start.\r\n\r\n");

    /*  
        Button initialization. 
        pin mode initialization.
        setting interrupts.
    */
    button.begin();

    /* Assigning the button functions */
    button.ButtonFunction1Address(device_button1_function);
    button.ButtonFunction2Address(device_button2_function);
    button.ButtonFunction3Address(device_button3_function);
    button.ButtonFunction4Address(device_button4_function);

    /* Turn status LED to On*/
    status.ledOn();
    while (true)
    {
        /* Checking if there is a button activation and execute it's assigned function */
        button.ButtonFunction1Process();
        button.ButtonFunction2Process();
        button.ButtonFunction3Process();
        button.ButtonFunction4Process();
        vTaskDelay(1);
    }
}

void device_relay_green_activate_task(void *p)
{ 
    RelayType_t rel; 
    Serial.printf("\r\n[Relay G Task] Device Relay Green Activate Task Start.\r\n\r\n");
    while (true)
    { 
        if (xQueueReceive(relayGreenQueueHandle, &rel, portMAX_DELAY))
        { 
            /* Take mutex to notify that the relay is activated */
            if (xSemaphoreTake(relayGreenMutex, 1000))
            { 
                /* Check if the relay is on pulse mode if yes then do the turn off sequence */
                if (relay.greenOffDelay() > 0)
                { 
                    /*  in this point the mutex is already take so it will just wait the alloted delay to expire.
                        also this will serve as key to turn off instantly the associated relay.  */
                    xSemaphoreTake(relayGreenOffMutex, relay.greenOffDelay());

                    /* Relay off */
                    relay.greenOff(); 
                    Serial.printf("[Relay G Task] Relay off.\r\n"); 
                }
                /* Clear bits for specific Event */
                xEventGroupClearBits(relayEvent, RELAY_GREEN_EVENT);
                /* Relase Mutex */
                xSemaphoreGive(relayGreenMutex);
            }
        }
    }
    vTaskDelete(NULL);
}

void device_relay_orange_activate_task(void *p)
{ 
    RelayType_t rel; 
    Serial.printf("\r\n[Relay O Task] Device Relay Orange Activate Task Start.\r\n\r\n");
    while (true)
    { 
        if (xQueueReceive(relayOrangeQueueHandle, &rel, 5000))
        { 
            /* Take mutex to notify that the relay is activated */
            if (xSemaphoreTake(relayOrangeMutex, 1000))
            { 
                /* Check if the relay is on pulse mode if yes then do the turn off sequence */
                if (relay.orangeOffDelay() > 0)
                {    
                    xSemaphoreTake(relayOrangeOffMutex, relay.orangeOffDelay()); 
                    relay.orangeOff(); 
                    Serial.printf("[Relay O Task] Relay off.\r\n"); 
                }
                xEventGroupClearBits(relayEvent, RELAY_ORANGE_EVENT);
                xSemaphoreGive(relayOrangeMutex); 
            }
        }
    }
    vTaskDelete(NULL);
}

void device_relay_red_activate_task(void *p)
{ 
    RelayType_t rel; 
    Serial.printf("\r\n[Relay R Task] Device Relay Orange Activate Task Start.\r\n\r\n");
    while (true)
    { 
        if (xQueueReceive(relayRedQueueHandle, &rel, 5000))
        { 
            /* Take mutex to notify that the relay is activated */
            if (xSemaphoreTake(relayRedMutex, 1000))
            {  
                /* Check if the relay is on pulse mode if yes then do the turn off sequence */
                if (relay.redOffDelay() > 0)
                {  
                    /*  in this point the mutex is already take so it will just wait the alloted delay to expire.
                        also this will serve as key to turn off instantly the associated relay.  */
                    xSemaphoreTake(relayRedOffMutex, relay.redOffDelay());

                    /* Relay off */ 
                    relay.redOff(); 
                    Serial.printf("[Relay R Task] Relay off.\r\n"); 
                }
                /* Clear bits for specific Event */
                xEventGroupClearBits(relayEvent, RELAY_RED_EVENT);
                /* Relase Mutex */
                xSemaphoreGive(relayRedMutex); 
            }
        }
    }
    vTaskDelete(NULL);
}

void device_relay_cancel_activate_task(void *p)
{ 
    RelayType_t rel; 
    Serial.printf("\r\n[Relay Cancel Task] Device Relay Cancel Activate Task Start.\r\n\r\n");
    while (true)
    {
        if (xQueueReceive(relayCancelQueueHandle, &rel, portMAX_DELAY))
        {
            /* Check if relay is still activated */
            if (xSemaphoreTake(relayCancelMutex, 1))
            {  
                /* Check if the relay is on pulse mode if yes then do the turn off sequence */
                if (relay.cancelOffDelay() > 0)
                {
                    /* Make sure GOR relay are off*/
                    relay.greenOff();
                    relay.orangeOff();
                    relay.redOff();

                    /*  in this point the mutex is already take so it will just wait the alloted delay to expire.
                        also this will serve as key to turn off instantly the associated relay.  */
                    xSemaphoreTake(relayCancelOffMutex, relay.cancelOffDelay());

                    /* Relay off */
                    relay.cancelOff();

                    /*  Make sure that GOR mutex is taken for the next turn off delay(PULSE) activation */
                    xSemaphoreTake(relayGreenOffMutex, 0);
                    xSemaphoreTake(relayOrangeOffMutex, 0);
                    xSemaphoreTake(relayRedOffMutex, 0);

                    Serial.printf("[Relay C Task] Relay off.\r\n"); 
                }
                /* Relase Mutex */
                xSemaphoreGive(relayCancelMutex);
            }
        }
    }
    vTaskDelete(NULL);
}

void device_relay_data_process_task(void *p)
{
    RelayType_t rel; 
    Serial.printf("\r\n[Relay Task] Device Relay Data Process Task Start.\r\n\r\n");
    while (true)
    { 
        /* Checking if there is available data on the queue and relay cancel mutex is available */
        if ((xQueuePeek(relayDelayOffQueueHandle, &rel, portMAX_DELAY)) && (xSemaphoreTake(relayCancelMutex, 100)))
        {
            uint32_t relayEventState = xEventGroupGetBits(relayEvent);
            Serial.printf("[Relay Task] EventState: %u\r\n",relayEventState); 

            /* Release Mutex */
            xSemaphoreGive(relayCancelMutex);

            /* get data from queue */
            xQueueReceive(relayDelayOffQueueHandle, &rel, portMAX_DELAY);

            /* Checking if what alert */
            if (rel == RELAY_CANCEL)
            {
                /* Release GOR mutex to activate turn off sequence */
                xSemaphoreGive(relayGreenOffMutex);
                xSemaphoreGive(relayOrangeOffMutex);
                xSemaphoreGive(relayRedOffMutex);
                /* Take the Cancel mutex */
                xSemaphoreTake(relayCancelOffMutex, 0);

                /* Turn on cancel relay */
                relay.cancelOn();

                /* Send queue to activate off sequence */
                xQueueSend(relayCancelQueueHandle, &rel, 500);
            }
            /* Checking if what alert and if the event is currently active */
            else if ((rel == RELAY_GREEN) && !(relayEventState & RELAY_GREEN_EVENT))
            {
                Serial.println("[Relay Task] Relay Green Activated."); 
                /* Turn on  relay */
                relay.greenOn();
                /* Set Event bits */
                xEventGroupSetBits(relayEvent, RELAY_GREEN_EVENT);
                /* Send queue to activate off sequence */
                xQueueSend(relayGreenQueueHandle, &rel, 500);
            }
            /* Checking if what alert and if the event is currently active */
            else if (rel == RELAY_ORANGE && !(relayEventState & RELAY_ORANGE_EVENT))
            {
                Serial.println("[Relay Task] Relay Orange Activated."); 
                /* Turn on  relay */
                relay.orangeOn();
                /* Set Event bits */
                xEventGroupSetBits(relayEvent, RELAY_ORANGE_EVENT);
                /* Send queue to activate off sequence */
                xQueueSend(relayOrangeQueueHandle, &rel, 500);
            }
            /* Checking if what alert and if the event is currently active */
            else if (rel == RELAY_RED && !(relayEventState & RELAY_RED_EVENT))
            {
                Serial.println("[Relay Task] Relay Red Activated."); 
                /* Turn on Red relay */
                relay.redOn();
                /* Set Event Red bits */
                xEventGroupSetBits(relayEvent, RELAY_RED_EVENT);
                /* Send queue to activate off sequence */
                xQueueSend(relayRedQueueHandle, &rel, 500);
            }
        }
    }
    vTaskDelete(NULL);
}



#ifndef WEBSERVER_OFF

/* Put IP Address details */
IPAddress local_ip(192, 168, 5, 1);
IPAddress gateway(192, 168, 5, 1);
IPAddress subnet(255, 255, 255, 0);

const char *PARAM_PROGRAM = "program";

AsyncWebServer server(80);

void web_not_found(AsyncWebServerRequest *request)
{ 
    Serial.println("Not Found.");
    request->redirect("http://" + WiFi.localIP().toString());
}
 
const char* http_username = "admin";
const char* http_password = "adminwrb";

void OnWiFiEvent(WiFiEvent_t event)
{
  switch (event) 
  { 
    case SYSTEM_EVENT_WIFI_READY:
            Serial.println("[WiFi Event] WRB Wi-Fi ready"); 
            break;
        case SYSTEM_EVENT_SCAN_DONE:
            Serial.println("[WiFi Event] WRB finish scanning AP");
            break; 
        case SYSTEM_EVENT_STA_START:
            Serial.println("[WiFi Event] WRB station start");
            break;
        case SYSTEM_EVENT_STA_STOP:
            Serial.println("[WiFi Event] WRB station stop");
            break; 
        case SYSTEM_EVENT_STA_CONNECTED:
            Serial.println("[WiFi Event] WRB Connected to WiFi Network"); 
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Serial.println("[WiFi Event] WRB station disconnected from AP"); 
            break; 
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
            Serial.println("[WiFi Event] WRB the auth mode of AP connected by WRB station changed");
            break; 
        case SYSTEM_EVENT_AP_START:
            Serial.println("[WiFi Event] WRB soft AP started");
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            Serial.println("[WiFi Event] Station connected to WRB soft AP");
            break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
            Serial.println("[WiFi Event] Station disconnected from WRB soft AP");
            break;
        default: 
            Serial.println("");
            break;
  }
}



void web_server(void)
{
    String chipMac = WiFi.macAddress();
    DeviceType_t info;

    /* Remove colon from the string */
    chipMac.replace(":", "");

    sprintf(device.WiFi.AP.SSID,"%s",chipMac.c_str());
    sprintf(device.WiFi.AP.Password,"%s",chipMac.substring(0, 8).c_str());
 
    Serial.printf("\r\n[Web Server] AP SSID: %s\n\n", device.WiFi.AP.SSID);
    
    
    WiFi.onEvent(OnWiFiEvent);
    /* Set WiFi mode to Access Point + Station */
    WiFi.mode(WIFI_AP_STA);
    /* Set Access Point Credentials */
    WiFi.softAP(device.WiFi.AP.SSID, device.WiFi.AP.Password); 
    vTaskDelay(100);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    
    /* Send web page with input fields to client */ 
    server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", "OK");
    });

    /* Config Webpage */
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!request->authenticate(http_username, http_password))
        return request->requestAuthentication();
        request->send_P(200, "text/html", config_html);
    });
    /* Homepage redirect to config page */
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) 
    {
        if(request->authenticate(http_username, http_password)) 
        {
            request->send_P(200, "text/html", config_html);
        }
        else
        {   
            request->requestAuthentication();
        }
    }); 

    /* This where configuration process done */
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) 
    {
        String inputMessage1, inputMessage2;

        String inputParam;
        
        if (request->hasParam(PARAM_PROGRAM))
        {
            inputMessage1 = request->getParam(PARAM_PROGRAM)->value();
            inputParam = PARAM_PROGRAM;
            Serial.println("[Web Server] Programming.");
            Serial.println(inputMessage1); 
            memset(dataSegment, '\0', sizeof(dataSegment));
            dataProcess((char *)inputMessage1.c_str(), dataSegment);
            dataPrint(dataSegment); 
            xSemaphoreGive(configurationMutex); 
            request->redirect("http://" + WiFi.localIP().toString());
        }
        else
        {
            inputMessage1 = "No message sent";
            inputParam = "none";
        }
        Serial.print("[Web Server] Message: ");
        Serial.println(inputMessage1);
        Serial.print("[Web Server] Param: ");
        Serial.println(inputParam);
    });

    /* Gettin all the information from the device to show it on the Config page */
    server.on("/configuration", HTTP_GET, [](AsyncWebServerRequest *request) 
    {
        
        char sensorData[1024];
        memset(sensorData, '\0', sizeof(sensorData));
        sprintf(sensorData, "%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%u|%u|%u|%u|%d%d%d%d|%1.1f|%s|%s|%s|%s|%s|%c",
                device.mqtt.IP,
                device.mqtt.port,
                device.ID.watch,
                device.ID.company,
                device.ID.location,
                device.WiFi.Credentials[0].SSID,
                device.WiFi.Credentials[1].SSID,
                device.WiFi.Credentials[2].SSID,
                device.WiFi.Credentials[3].SSID,
                device.WiFi.Credentials[4].SSID,
                device.WiFi.Credentials[0].Password,
                device.WiFi.Credentials[1].Password,
                device.WiFi.Credentials[2].Password,
                device.WiFi.Credentials[3].Password,
                device.WiFi.Credentials[4].Password,
                device.relay.green.group,
                device.relay.orange.group,
                device.relay.red.group,
                device.relay.cancel.group,
                device.relay.green.offTime,
                device.relay.orange.offTime,
                device.relay.red.offTime,
                device.relay.cancel.offTime,
                device.relay.cancel.onState,
                device.relay.red.onState,
                device.relay.orange.onState,
                device.relay.green.onState,
                FW_VERSION,
                device._static.IP,
                device._static.subnet, device._static.gateway,
                device._static.pDNS, device._static.sDNS,
                device._static.state[0]);

        request->send(200, "text/html", String(sensorData)); 
    }
    );
 
    server.onNotFound(web_not_found);

    /* Webserver begin */
    server.begin();
}
#endif
void device_mqtt_connection_task(void *p)
{
    
    uint8_t mqttState = 0x00;
    uint32_t hbCounter = 0x00;
    device.mqtt.state = true;
    Serial.printf("\r\n[MQTT Task] Device MQTT Connection Task Start.\r\n"); 
    Serial.println("---------------MQTT Payload init--------------");
    sprintf(device.mqtt.event.startUp.payload,"%s,%s,%s,Alarm,Unit,%s,%s,%s,%s"
            , device.ID.location
            , device.ID.company
            , device.ID.watch
            , DEVICE_TYPE
            , device.relay.green.group
            , device.relay.orange.group
            , device.relay.red.group);
    
    sprintf(device.mqtt.event.green.payload,"%s,%s,%s,Alarm,Unit,F,G,%s"
            , device.ID.location
            , device.ID.company
            , device.ID.watch 
            , device.relay.green.group);

    sprintf(device.mqtt.event.orange.payload,"%s,%s,%s,Alarm,Unit,F,O,%s"
            , device.ID.location
            , device.ID.company
            , device.ID.watch 
            , device.relay.green.group);
    
    sprintf(device.mqtt.event.red.payload,"%s,%s,%s,Alarm,Unit,F,R,%s"
            , device.ID.location
            , device.ID.company
            , device.ID.watch 
            , device.relay.green.group);

    sprintf(device.mqtt.event.cancel.payload,"%s,%s,%s,Alarm,Unit,F,C,0"
            , device.ID.location
            , device.ID.company
            , device.ID.watch);

    sprintf(device.mqtt.event.heartbeat.payload,"%s,%s,%s,H"
            , device.ID.location
            , device.ID.company
            , device.ID.watch); 
    
    Serial.println("---------------MQTT CREDENTIALS---------------");
    strcpy(device.mqtt.group, "0");
    sprintf(device.mqtt.commands.topic, "D/%s/%s/W/A", device.ID.company, device.ID.location);
    sprintf(device.mqtt.configuration.topic, "D/%s/%s/W/B", device.ID.company, device.ID.location);  
    sprintf(device.mqtt.event.startUp.topic,"S/%s",device.ID.company);
    sprintf(device.mqtt.event.green.topic,"D/%s",device.ID.company);
    sprintf(device.mqtt.event.orange.topic,"D/%s",device.ID.company);
    sprintf(device.mqtt.event.red.topic,"D/%s",device.ID.company);
    sprintf(device.mqtt.event.cancel.topic,"D/%s",device.ID.company);
    sprintf(device.mqtt.event.heartbeat.topic,"D/%s",device.ID.company);
    sprintf(device.mqtt.event.hbMSQ.topic,"D/%s/ALL/K/A",device.ID.company); 

    /* OTA Topic */ 
    sprintf(device.mqtt.event.firmwareUpdateByID.topic, "FW/%s", device.ID.watch);
    sprintf(device.mqtt.event.firmwareUpdateByCID.topic, "FW/%s", device.ID.company);
    sprintf(device.mqtt.event.firmwareUpdateByLID.topic, "FW/%s/%s", device.ID.company, device.ID.location);


    Serial.print("[MQTT Task] Packet Size: ");
    Serial.println(MQTT_MAX_PACKET_SIZE);

    Serial.print("[MQTT Task] IP:");
    Serial.println(device.mqtt.IP);
    Serial.print("[MQTT Task] Port: ");
    Serial.println(device.mqtt.port);
    mqtt.setServer(device.mqtt.IP, atoi(device.mqtt.port));
    mqtt.setCallback(device_mqtt_callback);
    
    while (pdTRUE)
    {
        /* Wait for the WiFi to be connected */
        if (WiFi.status() == WL_CONNECTED)
        {
            /* If the mqtt is not connected then do the reconnection */
            if(!mqtt.connected())
            {  
                /* Turn status LED to flashmode */ 
                deviceStartStatusLed(1000); 
                /* Make sure mqtt is disconnected */
                mqtt.disconnect();

                /* loop until mqtt is connected */
                while (!mqtt.connected())
                { 
                    Serial.printf("[MQTT Task] %s to MQTT.\r\n",(mqttState?"Reconnecting" : "Connecting")); 

                    /* Connect to the MQTT using the device ID */
                    if (mqtt.connect(device.ID.watch))
                    {
                        Serial.printf("[MQTT Task] MQTT Connected ID: %s\r\n",device.ID.watch);   
                        /* Set MQTT state */
                        mqttState = 0x01;
                        Serial.printf("[MQTT Task] Sending Startup Command.\r\n"); 

                        /* Publish the startup message */
                        mqtt.publish(device.mqtt.event.startUp.topic,device.mqtt.event.startUp.payload);
                        break;
                    }
                    else
                    {
                        Serial.printf("[MQTT Task] state: %d\r\n",mqtt.state());   
                    }
                    Serial.printf("[MQTT Task] WiFi Status: %d\r\n",WiFi.status()); 
                    vTaskDelay(5000);
                } 

                deviceStopStatusLed(); 

                Serial.printf("[MQTT Task] Connected to SSID: %s Status: %d\r\n",WiFi.SSID(),WiFi.status());  

                /* Printing all the topic that the WRB is connected */
                Serial.printf("[MQTT Task] Subscribe to: %s\r\n",device.mqtt.commands.topic);
                mqtt.subscribe(device.mqtt.commands.topic);
                Serial.printf("[MQTT Task] Subscribe to: %s\r\n",device.mqtt.configuration.topic);
                mqtt.subscribe(device.mqtt.configuration.topic);
                Serial.printf("[MQTT Task] Subscribe to: %s\r\n",device.mqtt.event.hbMSQ.topic);
                mqtt.subscribe(device.mqtt.event.hbMSQ.topic); 
                Serial.printf("[MQTT Task] Subscribe to: %s\r\n",device.mqtt.event.firmwareUpdateByID.topic);
                mqtt.subscribe(device.mqtt.event.firmwareUpdateByID.topic);  
                Serial.printf("[MQTT Task] Subscribe to: %s\r\n",device.mqtt.event.firmwareUpdateByCID.topic);
                mqtt.subscribe(device.mqtt.event.firmwareUpdateByCID.topic);  
                Serial.printf("[MQTT Task] Subscribe to: %s\r\n",device.mqtt.event.firmwareUpdateByLID.topic);
                mqtt.subscribe(device.mqtt.event.firmwareUpdateByLID.topic);  
            }
            else
            { 
                /* Check if heartbeat request receive from the server */
                if(device.hb.active == 0x01)
                {
                    /* Wait the hearbeat delay before publish heartbeat */
                    if(millis() >= device.hb.duration)
                    { 
                        /* Publish heartbeat */
                        boolean ret = mqtt.publish(device.mqtt.event.heartbeat.topic,device.mqtt.event.heartbeat.payload);
                        Serial.printf("\r\n[IO Task] HB Publish, %d, %s\r\n",ret,device.mqtt.event.heartbeat.payload);  
                        
                        /* Clear heatbeat status */
                        device.hb.active = 0x00;
                    }
                }
            }
        }
        /* keep this loop for the mqtt callback */
        mqtt.loop(); 
        vTaskDelay(1);  
    }
    vTaskDelete(NULL);
} 

void device_mqtt_connection_tasks(void *p)
{
    
    uint8_t mqttState = 0x00;
    uint32_t hbCounter = 0x00;
    device.mqtt.state = true;
    Serial.printf("\r\n[MQTT Task] Device MQTT Connection Task Start.\r\n"); 
    Serial.println("---------------MQTT Payload init--------------");
    sprintf(device.mqtt.event.startUp.payload,"%s,%s,%s,Alarm,Unit,%s,%s,%s,%s"
            , device.ID.location
            , device.ID.company
            , device.ID.watch
            , DEVICE_TYPE
            , device.relay.green.group
            , device.relay.orange.group
            , device.relay.red.group);
    
    sprintf(device.mqtt.event.green.payload,"%s,%s,%s,Alarm,Unit,F,G,%s"
            , device.ID.location
            , device.ID.company
            , device.ID.watch 
            , device.relay.green.group);

    sprintf(device.mqtt.event.orange.payload,"%s,%s,%s,Alarm,Unit,F,O,%s"
            , device.ID.location
            , device.ID.company
            , device.ID.watch 
            , device.relay.green.group);
    
    sprintf(device.mqtt.event.red.payload,"%s,%s,%s,Alarm,Unit,F,R,%s"
            , device.ID.location
            , device.ID.company
            , device.ID.watch 
            , device.relay.green.group);

    sprintf(device.mqtt.event.cancel.payload,"%s,%s,%s,Alarm,Unit,F,C,0"
            , device.ID.location
            , device.ID.company
            , device.ID.watch);

    sprintf(device.mqtt.event.heartbeat.payload,"%s,%s,%s,H"
            , device.ID.location
            , device.ID.company
            , device.ID.watch); 
    
    Serial.println("---------------MQTT CREDENTIALS---------------");
    strcpy(device.mqtt.group, "0");
    sprintf(device.mqtt.commands.topic, "D/%s/%s/W/A", device.ID.company, device.ID.location);
    sprintf(device.mqtt.configuration.topic, "D/%s/%s/W/B", device.ID.company, device.ID.location);  
    sprintf(device.mqtt.event.startUp.topic,"S/%s",device.ID.company);
    sprintf(device.mqtt.event.green.topic,"D/%s",device.ID.company);
    sprintf(device.mqtt.event.orange.topic,"D/%s",device.ID.company);
    sprintf(device.mqtt.event.red.topic,"D/%s",device.ID.company);
    sprintf(device.mqtt.event.cancel.topic,"D/%s",device.ID.company);
    sprintf(device.mqtt.event.heartbeat.topic,"D/%s",device.ID.company);
    sprintf(device.mqtt.event.hbMSQ.topic,"D/%s/ALL/K/A",device.ID.company); 

    /* 
        NOTE: Added OTA Topic for receiving URL for OTA update
    */ 
    sprintf(device.mqtt.event.firmwareUpdateByID.topic, "FW/%s", device.ID.watch);
    sprintf(device.mqtt.event.firmwareUpdateByCID.topic, "FW/%s", device.ID.company);
    sprintf(device.mqtt.event.firmwareUpdateByLID.topic, "FW/%s/%s", device.ID.company, device.ID.location);


    Serial.print("[MQTT Task] Packet Size: ");
    Serial.println(MQTT_MAX_PACKET_SIZE);

    Serial.print("[MQTT Task] IP:");
    Serial.println(device.mqtt.IP);
    Serial.print("[MQTT Task] Port: ");
    Serial.println(device.mqtt.port);
    mqtt.setServer(device.mqtt.IP, atoi(device.mqtt.port));
    mqtt.setCallback(device_mqtt_callback);
    
    while (pdTRUE)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            if(!mqtt.connected())
            {  
                deviceStartStatusLed(1000);
                mqtt.disconnect();
                while (!mqtt.connected())
                { 
                    Serial.printf("[MQTT Task] %s to MQTT.\r\n",(mqttState?"Reconnecting" : "Connecting")); 
                    if (mqtt.connect(device.ID.watch))
                    {
                        Serial.printf("[MQTT Task] MQTT Connected ID: %s\r\n",device.ID.watch);     
                        mqttState = 0x01;
                        Serial.printf("[MQTT Task] Sending Startup Command.\r\n"); 
                        mqtt.publish(device.mqtt.event.startUp.topic,device.mqtt.event.startUp.payload);
                        break;
                    }
                    else
                    {
                        Serial.printf("[MQTT Task] state: %d\r\n",mqtt.state());  

                    }
                    Serial.printf("[MQTT Task] WiFi Status: %d\r\n",WiFi.status()); 
                    vTaskDelay(5000);
                } 
                deviceStopStatusLed();

                Serial.printf("[MQTT Task] Connected to SSID: %s Status: %d\r\n",WiFi.SSID(),WiFi.status());  
 
                Serial.printf("[MQTT Task] Subscribe to: %s\r\n",device.mqtt.commands.topic);
                mqtt.subscribe(device.mqtt.commands.topic);
                Serial.printf("[MQTT Task] Subscribe to: %s\r\n",device.mqtt.configuration.topic);
                mqtt.subscribe(device.mqtt.configuration.topic);
                Serial.printf("[MQTT Task] Subscribe to: %s\r\n",device.mqtt.event.hbMSQ.topic);
                mqtt.subscribe(device.mqtt.event.hbMSQ.topic); 
                Serial.printf("[MQTT Task] Subscribe to: %s\r\n",device.mqtt.event.firmwareUpdateByID.topic);
                mqtt.subscribe(device.mqtt.event.firmwareUpdateByID.topic);  
                Serial.printf("[MQTT Task] Subscribe to: %s\r\n",device.mqtt.event.firmwareUpdateByCID.topic);
                mqtt.subscribe(device.mqtt.event.firmwareUpdateByCID.topic);  
                Serial.printf("[MQTT Task] Subscribe to: %s\r\n",device.mqtt.event.firmwareUpdateByLID.topic);
                mqtt.subscribe(device.mqtt.event.firmwareUpdateByLID.topic);  
            }
            else
            { 
                if(device.hb.active == 0x01)
                {
                    if(millis() >= device.hb.duration)
                    { 
                        boolean ret = mqtt.publish(device.mqtt.event.heartbeat.topic,device.mqtt.event.heartbeat.payload);
                        Serial.printf("\r\n[IO Task] HB Publish, %d, %s\r\n",ret,device.mqtt.event.heartbeat.payload);  
                        
                        device.hb.active = 0x00;
                    }
                }
            }
        }
        mqtt.loop(); 
        vTaskDelay(1);  
    }
    vTaskDelete(NULL);
} 

/*
    NOTE: newly implemented wifi reconnecting algorithm.
*/
void device_wifi_connection_task(void *p)
{
    Serial.printf("\r\n[WiFi Task] Device WiFi Connection Task Start.\r\n");
    if (device._static.state[0] == '1')
    {
        IPAddress ip, subnet, gateway, pDNS, sDNS;
        Serial.printf("[WiFi Task] Static ip configuration.\r\n");
        ip.fromString(device._static.IP);
        subnet.fromString(device._static.subnet);
        gateway.fromString(device._static.gateway);
        pDNS.fromString(device._static.pDNS);
        sDNS.fromString(device._static.sDNS);
        if (WiFi.config(ip, gateway, subnet, pDNS, sDNS) == false)
        {
            Serial.printf("[WiFi Task] Static ip configuration failed.\r\n");
        }
    }

    sprintf(device.WiFi.Credentials[5].SSID, "WRB_AP");
    sprintf(device.WiFi.Credentials[5].Password, "12345678");

    WiFi.setSleep(WIFI_PS_NONE);

    int indexWiFiCredentials = 0;
    int whileCounter = 40;
    int retryCounter = 10;
    bool wifiIsConnected = false;

    while (pdTRUE)
    {
        /* Loop until we're reconnected */
        if (WiFi.status() != WL_CONNECTED)
        {
            /* When the mqtt state is true then suspend the MQTT Task,
               while the WRB try to connect to wifi */
            if (device.mqtt.state == true)
            {
                Serial.printf("[WiFi Task] MQTT Task suspend.\r\n");
                vTaskSuspend(deviceMqttConnectionTaskHandle);
            }
            
            deviceStartStatusLed(250);
            WiFi.disconnect(true);
            WiFi.setAutoReconnect(false);
            deviceTimerStart(300000 / portTICK_PERIOD_MS, pdTRUE); 
            while (WiFi.status() != WL_CONNECTED)
            {
                for (int indexWiFiCredentials = 0; indexWiFiCredentials < 6; indexWiFiCredentials++)
                {
                    Serial.printf("[WiFi Task] Trying to connect to: %s | %s\r\n",device.WiFi.Credentials[indexWiFiCredentials].SSID,device.WiFi.Credentials[indexWiFiCredentials].Password);  
                    /* make sure that the SSID is valid as calling connect with a blank SSID can cause problems */ 
                    if (strlen(device.WiFi.Credentials[indexWiFiCredentials].SSID) >= 2)
                    {
                        whileCounter = 40;
                        WiFi.scanDelete();
                        vTaskDelay(5000);
                        int apCount = WiFi.scanNetworks(false, false, true, 300, 0, device.WiFi.Credentials[indexWiFiCredentials].SSID);
                        vTaskDelay(5000);
                        Serial.printf("[WiFi Task] Scanning Wi-Fi available. %d\r\n",apCount); 
                        /* we have identified the SSID is active therefore we can connect to it */
                        if (apCount >= 0x01)
                        {
                            WiFi.disconnect(true);
                            vTaskDelay(1000);
                            WiFi.begin(device.WiFi.Credentials[indexWiFiCredentials].SSID, device.WiFi.Credentials[indexWiFiCredentials].Password);
                            /* Waiting for the WiFi status to be connected then exit loop when the counter expires */
                            while ((WiFi.status() != WL_CONNECTED) && (whileCounter > 0))
                            { 
                                vTaskDelay(1000); 
                                whileCounter--;
                            }
                            /* When the WiFi status is connected then break the loop */
                            if(WiFi.status() == WL_CONNECTED)
                            {
                                break;
                            } 
                        }
                    }
                }

                /* clear the wifi scan result to save memory and ensure that its not accidentally used next time */
                WiFi.scanDelete();

                vTaskDelay(5000);

                Serial.printf("[WiFi Task] Reconnection to WiFi | Status Code: %d\n", WiFi.status()); 
                if (retryCounter-- <= 0) 
                { 
                    Serial.printf("[WiFi Task] WRB Restart\r\n"); 
                    vTaskDelay(100);
                    /* really need to consider that the wifi is stuck so reboot and try again */
                    ESP.restart(); 
                }
            }

            deviceTimerStop();

            deviceStopStatusLed(); 


            Serial.printf("\r\n\r\n[WiFi Task] WiFi Connection Details:\r\n[WiFi Task] SSID: %s\r\n[WiFi Task] Status: %s | Code: %d\r\n",
                          WiFi.SSID().c_str(),
                          (WiFi.status() == WL_CONNECTED ? "Connected" : "Other"), WiFi.status()); 

            Serial.printf("[WiFi Task] IP Address Local: %s | Broadcast: %s\r\n\r\n"
                          ,WiFi.localIP().toString().c_str(), WiFi.broadcastIP().toString().c_str());
             

            /* When the mqtt state is true then resume the MQTT Task */
            if (device.mqtt.state == true)
            {
                vTaskResume(deviceMqttConnectionTaskHandle);
            }
        }

        vTaskDelay(1);
    }

    WiFi.setAutoReconnect(true);

    vTaskDelete(NULL);
}

void device_configuration_Task(void *p)
{
    Serial.printf("\r\n[Config Task] Device Coniguration Task Start.\r\n\r\n");
    vSemaphoreCreateBinary(configurationMutex);
    xSemaphoreTake(configurationMutex, 0);


    while (pdTRUE)
    {
        /* Wait for the configuration mutex available then do the configuration*/
        if (xSemaphoreTake(configurationMutex, portMAX_DELAY))
        {
            Serial.println("[Config Task] xSemaphoreTake."); 
            /* Print receive configuration data*/
            dataPrint(dataSegment);

            /* Sort all information from the configuration data*/
            sortInformation(&device, dataSegment);

            /* Store all infromation to the database */
            setDeviceInformation(&device);

            /* Print the stored information for debugging purporse */
            printInformation(&device);

            /* Turn off status LED to tell the user that configuration success and the device will restart */ 
            status.ledOff(); 

            Serial.print("\r\n\r\n[Config Task] System Restarting");
            for (uint8_t index = 0; index < 5; index++)
            {
                Serial.print(".");
                vTaskDelay(1000);
            }
            Serial.println("\r\n\r\n\r\n");

            /* Restart the device to apply the new configuration*/
            ESP.restart();
        } 
    }
    vTaskDelete(NULL);
}

void device_event_group(void)
{
    /* Event Group Create */
    relayEvent = xEventGroupCreate(); 
}

void device_queue(void)
{
    /* Queue Handle Create */
    relayDelayOffQueueHandle = xQueueCreate(20, sizeof(RelayType_t));
    relayGreenQueueHandle = xQueueCreate(20, sizeof(RelayType_t));
    relayOrangeQueueHandle = xQueueCreate(20, sizeof(RelayType_t));
    relayRedQueueHandle = xQueueCreate(20, sizeof(RelayType_t));
    relayCancelQueueHandle = xQueueCreate(20, sizeof(RelayType_t)); 
}

void device_semaphore(void)
{
    /* Semaphore Create Mutex */
    relayDelayOffMutex = xSemaphoreCreateMutex();
    relayGreenMutex = xSemaphoreCreateMutex();
    relayOrangeMutex = xSemaphoreCreateMutex();
    relayRedMutex = xSemaphoreCreateMutex();
    relayCancelMutex = xSemaphoreCreateMutex();

    /* Semaphore Create Binary */
    vSemaphoreCreateBinary(relayGreenOffMutex);
    vSemaphoreCreateBinary(relayOrangeOffMutex);
    vSemaphoreCreateBinary(relayRedOffMutex);
    vSemaphoreCreateBinary(relayCancelOffMutex); 
    xSemaphoreTake(relayGreenOffMutex, 0);
    xSemaphoreTake(relayOrangeOffMutex, 0);
    xSemaphoreTake(relayRedOffMutex, 0); 
}

void device_sub_task(void)
{
    /*
        NOTE: Change the core from APP_CPU_NUM(1) to PRO_CPU_NUM(0)
    */
    xTaskCreatePinnedToCore(device_relay_data_process_task,     "DeviceRelayDataProcess",       2024, NULL, DEVICE_TASKPRIORITIES_RELAY_DATA_PROCESS,       &deviceRelayDataProcessTaskHandle,  PRO_CPU_NUM);
    xTaskCreatePinnedToCore(device_relay_green_activate_task,   "DeviceRelayGreenActivate",     2024, NULL, DEVICE_TASKPRIORITIES_RELAY_GREEN_ACTIVATE,     &deviceRelayGreenTaskHandle,        PRO_CPU_NUM);
    xTaskCreatePinnedToCore(device_relay_orange_activate_task,  "DeviceRelayOrangeActivate",    2024, NULL, DEVICE_TASKPRIORITIES_RELAY_ORANGE_ACTIVATE,    &deviceRelayOrangeTaskHandle,       PRO_CPU_NUM);
    xTaskCreatePinnedToCore(device_relay_red_activate_task,     "DeviceRelayRedActivate",       2024, NULL, DEVICE_TASKPRIORITIES_RELAY_RED_ACTIVATE,       &deviceRelayRedTaskHandle,          PRO_CPU_NUM);
    xTaskCreatePinnedToCore(device_relay_cancel_activate_task,  "DeviceRelayRedActivate",       2024, NULL, DEVICE_TASKPRIORITIES_RELAY_CANCEL_ACTIVATE,    &deviceRelayCancelTaskHandle,       PRO_CPU_NUM);
} 

/*
    NOTE: Newly added task. This task will be responsible for the OTA of the WRB
*/
void device_OTA_Task(void *pvParameter)
{
    char deviceOTAMQTTMessageBuffer[DEVICE_OTA_MQTT_MSG_LENGTH];
    
    deviceOTAQueueHandle = xQueueCreate(1,sizeof(deviceOTAMQTTMessageBuffer));
    Serial.printf("\r\n[OTA TASK] Device OTA Task Start.\r\n\r\n");
    while (pdTRUE)
    {
        /* Wait Queue for the server for the URL of the binary for OTA.*/
        if (xQueueReceive(deviceOTAQueueHandle, deviceOTAMQTTMessageBuffer, portMAX_DELAY))
        {
            /* Make sure that Wifi is connected during the OTA */
            if (WiFi.status() == WL_CONNECTED)
            {
                WiFiClient client;
                Serial.printf("[OTA TASK] Bin File Web Address: %s\r\n", deviceOTAMQTTMessageBuffer);   

                /* here where the downloading of the file and firmware upgrade occur */
                t_httpUpdate_return ret = httpUpdate.update(client, String(deviceOTAMQTTMessageBuffer)); 
                switch (ret)
                {
                case HTTP_UPDATE_FAILED: 
                    Serial.printf("[OTA TASK] HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str()); 
                    break;

                case HTTP_UPDATE_NO_UPDATES: 
                    Serial.println("[OTA TASK] HTTP_UPDATE_NO_UPDATES"); 
                    break;

                case HTTP_UPDATE_OK: 
                    Serial.println("[OTA TASK] HTTP_UPDATE_OK"); 
                    break;
                } 
            } 
        }
    }
}

void device_init_task(void)
{
    Serial.begin(115200);
    Serial.printf("\r\n[Device init] Device Task Init Start.\r\n");
    Serial.printf("\r\n[Device init] Firmware Version: %1.1fv\r\n",FW_VERSION);
    device_check_database(); 

    
    device_event_group();
    device_queue();
    device_semaphore();

    relay.begin( device.relay.green.onState, device.relay.orange.onState, device.relay.red.onState, device.relay.cancel.onState,
                 device.relay.green.offTime, device.relay.orange.offTime, device.relay.red.offTime, device.relay.cancel.offTime);   
 
    #ifndef WEBSERVER_OFF
      web_server();
    #endif
    /* 
        NOTE:   Change the core from APP_CPU_NUM(1) to PRO_CPU_NUM(0) of 
                    device_configuration_Task
                    device_button_task
                Remove the configuration window time.
                Remove Serial Task.
                Remove LED status task and use software timer of freeRTOS for blinking.
                Added OTA Task
     */
    xTaskCreatePinnedToCore(device_configuration_Task, "DeviceConfiguration", 8192, NULL, DEVICE_TASKPRIORITIES_CONFIGURATION, &deviceConfigurationTaskHandle, PRO_CPU_NUM); 
    if(strcmp("c00000",device.ID.company) != 0x00)
    {   
        xTaskCreatePinnedToCore(device_wifi_connection_task, "DeviceWiFi", 8192, NULL, DEVICE_TASKPRIORITIES_WIFI, &deviceWifiConnectionTaskHandle, APP_CPU_NUM);  
        xTaskCreatePinnedToCore(device_mqtt_connection_task, "DeviceMQTT", 8192, NULL, DEVICE_TASKPRIORITIES_MQTT, &deviceMqttConnectionTaskHandle, APP_CPU_NUM);
        xTaskCreatePinnedToCore(device_button_task, "DeviceButton", 4096, NULL, DEVICE_TASKPRIORITIES_BUTTON, &deviceButtonTaskHandle, APP_CPU_NUM);
        device_sub_task();
        xTaskCreatePinnedToCore(device_OTA_Task, "DeviceOTA", 4096, NULL, DEVICE_TASKPRIORITIES_DEVICEOTA, &deviceOTATaskHandle,APP_CPU_NUM);
  
    }   
}   


