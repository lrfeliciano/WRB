#include <DeviceTask.h>

#include <Cryptography.h>
#include <ESPAsyncWebServer.h>
#include <freertos/FreeRTOSConfig.h>

#define DEVICE_NORMAL_OPERATION 

/* GREEN RELAY BUTTON */
#define DEVICE_BUTTON_1 34 
/* ORANGE RELAY BUTTON */
#define DEVICE_BUTTON_2 35  
/* CANCEL RELAY BUTTON */
#define DEVICE_BUTTON_3 33
/* RED RELAY BUTTON */
#define DEVICE_BUTTON_4 32

#define DEVICE_STATUS_LED   18

Relay relay(RELAY_1_PIN, RELAY_2_PIN, RELAY_3_PIN, RELAY_4_PIN);
Button button(DEVICE_BUTTON_1, DEVICE_BUTTON_2, DEVICE_BUTTON_3, DEVICE_BUTTON_4);
Status status(DEVICE_STATUS_LED);


char dataSegment[COMMAND_SET_MAX][49];

WiFiClient wrbWifi;
PubSubClient client(wrbWifi);

DeviceType_t device;

#ifdef DEVICE_NORMAL_OPERATION
/* Task Handle Declaration */
TaskHandle_t deviceWifiConnectionTaskHandle;
TaskHandle_t deviceMqttConnectionTaskHandle;
TaskHandle_t deviceConfigurationTaskHandle;
TaskHandle_t deviceButtonTaskHandle;
TaskHandle_t deviceLedStatusTaskHandle;
TaskHandle_t deviceSerialTaskHandle;

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
EventGroupHandle_t ledStatusEvent;



#define LED_STATUS_EVENT_WIFI_CONNECTING        0x01
#define LED_STATUS_EVENT_MQTT_CONNECTING        0x02
#define LED_STATUS_EVENT                        LED_STATUS_EVENT_WIFI_CONNECTING | LED_STATUS_EVENT_MQTT_CONNECTING

#define LED_STATUS_WIFI_ON_TIME     100
#define LED_STATUS_WIFI_OFF_TIME    100

#define LED_STATUS_MQTT_ON_TIME     1000
#define LED_STATUS_MQTT_OFF_TIME    1000

#define WIFI_MULTI 

void status_blink_task(void *p)
{  
    status.ledOff();
    vTaskDelay(100 / portTICK_PERIOD_MS);
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
    Serial.printf("Topic: %s\r\n", topic);

    if(!strcmp(topic,device.mqtt.event.hbMSQ.topic))
    {
        client.publish(device.mqtt.event.heartbeat.topic,device.mqtt.event.heartbeat.payload);
    }

    if(!strcmp(topic,device.mqtt.commands.topic)) 
    {
        RelayType_t rel;
        Serial.print("Message: ");
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
            }
            else if (index == 1)
            {
                LID[posIndex] = payload[i];
            }
            else if (index == 2)
            {
                alertType[posIndex] = payload[i];
            }
            else if (index == 3)
            {
                groupCode[posIndex] = payload[i]; 
            }
        }

        Serial.print("\r\n• CID: ");
        Serial.println(CID);
        Serial.print("• LID: ");
        Serial.println(LID);
        Serial.print("• AT: ");
        Serial.println(alertType);
        Serial.print("• GC: ");
        Serial.println(groupCode);

        if (!strcmp(alertType, "C"))  
        {
            if (strchr(device.relay.cancel.group, (int)groupCode[0]) != 0)
            {
                rel = RELAY_4;
                Serial.println("Cancel Semaphore give.");
            }
        }
        else if ((!strcmp(alertType, "G")) && (xSemaphoreTake(relayGreenMutex, 1)))
        {
            if (strchr(device.relay.green.group, (int)groupCode[0]) != 0)
            {
                rel = RELAY_GREEN;
                Serial.println("Green Semaphore give.");
                xSemaphoreGive(relayGreenMutex);
            }
        }
        else if ((!strcmp(alertType, "O")) && (xSemaphoreTake(relayOrangeMutex, 1)))
        {
            if (strchr(device.relay.orange.group, (int)groupCode[0]) != 0)
            {
                rel = RELAY_ORANGE;
                Serial.println("Orange Semaphore give.");
                xSemaphoreGive(relayOrangeMutex);
            }
        }
        else if (!strcmp(alertType, "R") && (xSemaphoreTake(relayRedMutex, 1)))
        {
            if (strchr(device.relay.red.group, (int)groupCode[0]) != 0)
            {
                rel = RELAY_RED;
                Serial.println("Red Semaphore give.");
                xSemaphoreGive(relayRedMutex);
            }
        }

        if (rel > RELAY_NONE)
        { 
            xQueueSend(relayDelayOffQueueHandle, &rel, 500);
            Serial.print("Command ");
            Serial.print(rel);
            Serial.println(" Queued!!!");
        }
    } 
    else if (!strcmp(topic, device.mqtt.configuration.topic))
    {
        Serial.print("Message:");
        for (int i = 0; i < length; i++, posIndex++)
        { 
            Serial.print((char)payload[i]); 
        }
        Serial.println();
        memset(dataSegment, '\0', sizeof(dataSegment));
        dataProcess((char *)payload, dataSegment); 
        xSemaphoreGive(configurationMutex);
    }
    Serial.println("----------------------------------------------\r\n");
}

#endif 
void device_check_database(void)
{
    Serial.println("----------------DEVICE CHECK DATABASE---------------"); 
    String MAC; 
    
    memset(&device, '\0', sizeof(device));
    device.MAC = WiFi.macAddress();
    MAC = device.MAC;
    device.MAC.replace(":", ""); 
    Serial.println("\r\nDevice Details:");
    Serial.print("Host: ");
    Serial.println(device.IPAddress);
    Serial.print("SSID: ");
    Serial.println(device.SSID);
    Serial.print("MAC: ");
    Serial.println(MAC);
    device.MAC.toLowerCase();
    Serial.print("MAC Low key: ");
    Serial.println(device.MAC);
    Serial.print("MAC RAW: ");
    Serial.println(device.MAC);
    
    if(deviceDatabaseInit() & 0x02)
    {
        sprintf(device.ID.watch, "b000%s", device.MAC.c_str()); 
        Serial.print("New Watch ID: ");
        Serial.println(device.ID.watch);

        deviceDatabaseSave();
    }
    
    
    getDeviceInformation(&device);
    if(!strcmp(device.ID.watch,""))
    {
        sprintf(device.ID.watch, "b000%s", device.MAC.c_str()); 
        Serial.print("New Watch ID: ");
        Serial.println(device.ID.watch);
        setDeviceInformation(&device);
    } 
    Serial.println("----------------------------------------------\r\n");  
}

#ifdef DEVICE_NORMAL_OPERATION
void prints(void)
{
    Serial.println("void print(void) An interrupt has occurred. Total: ");
}

const long buttonInterval = 500;

void device_button1_function(void)
{
    uint8_t pressCounter = 0;
    unsigned long prevTime = millis(); 

    while (button.Button1Read())
    {
        unsigned long curTime = millis();
        if ((curTime - prevTime) >= buttonInterval)
        {
            prevTime = curTime;
            pressCounter++; 
            xTaskCreate(status_blink_task, "StatusBlink", 1024, NULL, 10, &statusBinkTaskHandle);
        }

        if(pressCounter >=  10)
        { 
            client.publish(device.mqtt.event.cancel.topic,device.mqtt.event.cancel.payload);
            break;
        }
    } 

    pressCounter = 0;
}

void device_button2_function(void)
{
    uint8_t pressCounter = 0;
    unsigned long prevTime = millis(); 
    while (button.Button2Read() == button.Button2State())
    {
        unsigned long curTime = millis();
        if ((curTime - prevTime) >= buttonInterval)
        {
            prevTime = curTime;
            pressCounter++;
            xTaskCreate(status_blink_task, "StatusBlink", 1024, NULL, 10, &statusBinkTaskHandle); 
        }
        if(pressCounter >=  10)
        { 
            if(!button.Button2State())
            {
                client.publish(device.mqtt.event.cancel.topic,device.mqtt.event.cancel.payload);
            }
            else
            {
                client.publish(device.mqtt.event.red.topic,device.mqtt.event.red.payload);
            }
            Serial.printf("%s Alert\n\n",button.Button2State() ? "Cancel" : "Red");
            button.Button2InvertState();
            break;
        }
    } 
    pressCounter = 0;
}

void device_button3_function(void)
{
    uint8_t pressCounter = 0;
    unsigned long prevTime = millis(); 
 
    while (button.Button3Read() == button.Button3State())
    {
        unsigned long curTime = millis();
        if ((curTime - prevTime) >= buttonInterval)
        {
            prevTime = curTime;
            pressCounter++;
            xTaskCreate(status_blink_task, "StatusBlink", 1024, NULL, 10, &statusBinkTaskHandle); 
        }
        if(pressCounter >=  10)
        { 
            if(button.Button3State())
            {
                client.publish(device.mqtt.event.cancel.topic,device.mqtt.event.cancel.payload);
            }
            else
            {
                client.publish(device.mqtt.event.red.topic,device.mqtt.event.red.payload);
            }
            Serial.printf("%s Alert\n\n",button.Button3State() ? "Cancel" : "Red");
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
     
    while (!button.Button4Read())
    {
        unsigned long curTime = millis();
        if ((curTime - prevTime) >= buttonInterval)
        {
            prevTime = curTime;
            pressCounter++; 
            xTaskCreate(status_blink_task, "StatusBlink", 1024, NULL, 10, &statusBinkTaskHandle);
        }

        if(pressCounter >=  10)
        { 
            client.publish(device.mqtt.event.cancel.topic,device.mqtt.event.cancel.payload);
            break;
        }
    }  
    pressCounter = 0;
}

void device_button_task(void *p)
{
#ifdef SHOW_CORE
    Serial.print("buttonTask running on core: ");
    Serial.println(xPortGetCoreID());
#endif
    Serial.println("buttonTask Created.");
    Serial.println("Monitoring interrupts: ");
    button.begin();

    button.ButtonFunction1Address(device_button1_function);
    button.ButtonFunction2Address(device_button2_function);
    button.ButtonFunction3Address(device_button3_function);
    button.ButtonFunction4Address(device_button4_function);
    status.ledOn();
    while (true)
    {
        button.ButtonFunction1Process();
        button.ButtonFunction2Process();
        button.ButtonFunction3Process();
        button.ButtonFunction4Process();
        delay(1);
    }
}

void device_relay_green_activate_task(void *p)
{ 
    RelayType_t rel; 
    Serial.printf("\r\n\t\t\t\t\tDevice Relay Green Activate Task Start.\r\n\r\n");
    while (true)
    { 
        if (xQueueReceive(relayGreenQueueHandle, &rel, portMAX_DELAY))
        { 
            if (xSemaphoreTake(relayGreenMutex, 1000))
            { 
                if (relay.greenOffDelay() > 0)
                { 
                    xSemaphoreTake(relayGreenOffMutex, relay.greenOffDelay());
                    relay.greenOff(); 
                }
                xEventGroupClearBits(relayEvent, RELAY_GREEN_EVENT);
                xSemaphoreGive(relayGreenMutex);
            }
        }
    }
    vTaskDelete(NULL);
}

void device_relay_orange_activate_task(void *p)
{ 
    RelayType_t rel; 
    Serial.printf("\r\n\t\t\t\t\tDevice Relay Orange Activate Task Start.\r\n\r\n");
    while (true)
    { 
        if (xQueueReceive(relayOrangeQueueHandle, &rel, 5000))
        {
            if (xSemaphoreTake(relayOrangeMutex, 1000))
            {
                if (relay.orangeOffDelay() > 0)
                { 
                    xSemaphoreTake(relayOrangeOffMutex, relay.orangeOffDelay());
                    relay.orangeOff(); 
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
    Serial.printf("\r\n\t\t\t\t\tDevice Relay Red Activate Task Start.\r\n\r\n");
    while (true)
    { 
        if (xQueueReceive(relayRedQueueHandle, &rel, 5000))
        {
            if (xSemaphoreTake(relayRedMutex, 1000))
            { 
                if (relay.redOffDelay() > 0)
                { 
                    xSemaphoreTake(relayRedOffMutex, relay.redOffDelay());
                    relay.redOff(); 
                }
                xEventGroupClearBits(relayEvent, RELAY_RED_EVENT);
                xSemaphoreGive(relayRedMutex);
            }
        }
    }
    vTaskDelete(NULL);
}

void device_relay_cancel_activate_task(void *p)
{ 
    RelayType_t rel; 
    Serial.printf("\r\n\t\t\t\t\tDevice Relay Cancel Activate Task Start.\r\n\r\n");
    while (true)
    {
        if (xQueueReceive(relayCancelQueueHandle, &rel, portMAX_DELAY))
        {
            if (xSemaphoreTake(relayCancelMutex, 1))
            { 
                Serial.println(relay.cancelOffDelay());
                if (relay.cancelOffDelay() > 0)
                {
                    relay.greenOff();
                    relay.orangeOff();
                    relay.redOff();
                    xSemaphoreTake(relayCancelOffMutex, relay.cancelOffDelay());
                    relay.cancelOff();
                    xSemaphoreTake(relayGreenOffMutex, 0);
                    xSemaphoreTake(relayOrangeOffMutex, 0);
                    xSemaphoreTake(relayRedOffMutex, 0);
                }
                xSemaphoreGive(relayCancelMutex);
            }
        }
    }
    vTaskDelete(NULL);
}

void device_relay_data_process_task(void *p)
{
    RelayType_t rel, r; 
    Serial.printf("\r\n\t\t\t\t\tDevice Relay Data Process Task Start.\r\n\r\n");
    while (true)
    { 
        if ((xQueuePeek(relayDelayOffQueueHandle, &r, portMAX_DELAY)) && (xSemaphoreTake(relayCancelMutex, 100)))
        {
            uint32_t relayEventState = xEventGroupGetBits(relayEvent);
            Serial.print("relayEventState: ");
            Serial.println(relayEventState);
            xSemaphoreGive(relayCancelMutex);
            xQueueReceive(relayDelayOffQueueHandle, &rel, portMAX_DELAY);

            if (rel == RELAY_CANCEL)
            {
                xSemaphoreGive(relayGreenOffMutex);
                xSemaphoreGive(relayOrangeOffMutex);
                xSemaphoreGive(relayRedOffMutex);
                xSemaphoreTake(relayCancelOffMutex, 0);
                relay.cancelOn();

                xQueueSend(relayCancelQueueHandle, &rel, 500);
            }
            else if ((rel == RELAY_GREEN) && !(relayEventState & RELAY_GREEN_EVENT))
            {
                Serial.println("Relay Green Activated.");
                relay.greenOn();
                xEventGroupSetBits(relayEvent, RELAY_GREEN_EVENT);
                xQueueSend(relayGreenQueueHandle, &rel, 500);
            }
            else if (rel == RELAY_ORANGE && !(relayEventState & RELAY_ORANGE_EVENT))
            {
                Serial.println("Relay Orange Activated.");
                relay.orangeOn();
                xEventGroupSetBits(relayEvent, RELAY_ORANGE_EVENT);
                xQueueSend(relayOrangeQueueHandle, &rel, 500);
            }
            else if (rel == RELAY_RED && !(relayEventState & RELAY_RED_EVENT))
            {
                Serial.println("Relay Red Activated.");
                relay.redOn();
                xEventGroupSetBits(relayEvent, RELAY_RED_EVENT);
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

#endif

#ifndef WEBSERVER_OFF
const char* http_username = "admin";
const char* http_password = "adminwrb";

void OnWiFiEvent(WiFiEvent_t event)
{
  switch (event) {
 
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("\nESP32 Connected to WiFi Network\n");
      break;
    case SYSTEM_EVENT_AP_START:
      Serial.println("\nESP32 soft AP started\n");
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      Serial.println("\nStation connected to ESP32 soft AP\n");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      Serial.println("\nStation disconnected from ESP32 soft AP\n");
      break;
    default: break;
  }
}

void web_server(void)
{
    String chipMac = WiFi.macAddress();
    DeviceType_t info;
    chipMac.replace(":", "");

    sprintf(device.WiFi.AP.SSID,"%s",chipMac.c_str());
    sprintf(device.WiFi.AP.Password,"%s",chipMac.substring(0, 8).c_str());
 
    Serial.printf("\n\nAP SSID: %s\n\n", device.WiFi.AP.SSID);
    WiFi.onEvent(OnWiFiEvent);
    WiFi.mode(WIFI_AP_STA);
    
    WiFi.softAP(device.WiFi.AP.SSID, device.WiFi.AP.Password); 
    vTaskDelay(100 / portTICK_PERIOD_MS);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    
    // Send web page with input fields to client
    server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", "OK");
    });
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!request->authenticate(http_username, http_password))
        return request->requestAuthentication();
        request->send_P(200, "text/html", config_html);
    });

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

    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) 
    {
        String inputMessage1, inputMessage2;

        String inputParam;
        
        if (request->hasParam(PARAM_PROGRAM))
        {
            inputMessage1 = request->getParam(PARAM_PROGRAM)->value();
            inputParam = PARAM_PROGRAM;
            Serial.println("Programming.");
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
        Serial.print("Message: ");
        Serial.println(inputMessage1);
        Serial.print("Param: ");
        Serial.println(inputParam);
    });

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
    server.begin();
}
#endif

void device_mqtt_connection_task(void *p)
{
    
    uint8_t mqttState = 0x00;
    uint32_t hbCounter = 0x00;
    device.mqtt.state = true;
    Serial.printf("\r\n\t\t\t\t\tDevice MQTT Connection Task Start.\r\n\r\n"); 
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

    Serial.print("Packet Size: ");
    Serial.println(MQTT_MAX_PACKET_SIZE);

    Serial.print("IP:");
    Serial.println(device.mqtt.IP);
    Serial.print("Port: ");
    Serial.println(device.mqtt.port);
    client.setServer(device.mqtt.IP, atoi(device.mqtt.port));
    client.setCallback(device_mqtt_callback);
    
    while (pdTRUE)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            if(!client.connected())
            {  
                xEventGroupSetBits(ledStatusEvent,LED_STATUS_EVENT_MQTT_CONNECTING);
                client.disconnect();
                while (!client.connected())
                { 
                    Serial.printf("%s to MQTT.",(mqttState?"Reconnecting" : "Connecting"));
                    if (client.connect(device.ID.watch))
                    {
                        Serial.println("MQTT Connected");
                        Serial.print("MQTT ID: ");
                        Serial.println(device.ID.watch);
                        mqttState = 0x01;
                        Serial.printf("Sending Startup Command.");
                        client.publish(device.mqtt.event.startUp.topic,device.mqtt.event.startUp.payload);
                        break;
                    }
                    else
                    {
                        Serial.printf("MQTT state: %d\r\n",client.state()); 

                    }
                    Serial.printf("WiFi Status: %d\r\n",WiFi.status());
                    vTaskDelay(5000/portTICK_PERIOD_MS);
                } 
                xEventGroupClearBits(ledStatusEvent,LED_STATUS_EVENT_MQTT_CONNECTING);
                status.ledOn(); 

                Serial.println("Connected to:");
                Serial.print("SSID: ");
                Serial.println(WiFi.SSID());
                Serial.print("WiFi Connection Status: ");
                Serial.println(WiFi.status());

 
                Serial.printf("Subscribe to: %s\r\n",device.mqtt.commands.topic);
                client.subscribe(device.mqtt.commands.topic);
                Serial.printf("Subscribe to: %s\r\n",device.mqtt.configuration.topic);
                client.subscribe(device.mqtt.configuration.topic);
                Serial.printf("Subscribe to: %s\r\n",device.mqtt.event.hbMSQ.topic);
                client.subscribe(device.mqtt.event.hbMSQ.topic);
                client.subscribe("WRB/HB");  
            }
            else
            {
                if(hbCounter >= DEVICE_HEART_BEAT)
                {
                    Serial.printf("Sending Heartbeat.");
                    client.publish("WRB/HB",device.mqtt.event.heartbeat.payload); 
                    hbCounter = 0;
                } 
            }
        }
        client.loop(); 
        delay(1); 
        hbCounter++;
    }
    vTaskDelete(NULL);
}

void device_wifi_connection_task(void *p)
{
    Serial.printf("\r\n\t\t\t\t\tDevice WiFi Connection Task Start.\r\n\r\n");  
    
    if(device._static.state[0] == '1')
    {
        IPAddress ip,subnet,gateway,pDNS,sDNS;
        Serial.printf("Static ip configuration.\r\n");
        ip.fromString(device._static.IP);
        subnet.fromString(device._static.subnet);
        gateway.fromString(device._static.gateway);
        pDNS.fromString(device._static.pDNS);
        sDNS.fromString(device._static.sDNS); 
       if(WiFi.config(ip,gateway,subnet,pDNS,sDNS) == false)
       {
           Serial.printf("\t\t\t\t\tStatic ip configuration failed.\r\n");
       }
    }
    
    sprintf(device.WiFi.Credentials[5].SSID,"WRB_AP");
    sprintf(device.WiFi.Credentials[5].Password,"12345678"); 

    while (pdTRUE)
    {
        // Loop until we're reconnected
        if (WiFi.status() != WL_CONNECTED)
        {
            if(device.mqtt.state == true)
            {
                vTaskSuspend(deviceMqttConnectionTaskHandle);
            }
            
            xEventGroupSetBits(ledStatusEvent,LED_STATUS_EVENT_WIFI_CONNECTING);
             
            WiFi.disconnect(); 
            deviceTimerStart(300000/portTICK_PERIOD_MS, pdTRUE); 
            while (WiFi.status() != WL_CONNECTED) 
            {
               
                int apCount = WiFi.scanNetworks();
                for (int apIndex = 0; apIndex < apCount; apIndex++) 
                { 
                    for(int indexWiFiCredentials = 0; indexWiFiCredentials < 6; indexWiFiCredentials++)
                    {
                        if(strcmp(WiFi.SSID(apIndex).c_str(),device.WiFi.Credentials[indexWiFiCredentials].SSID) == 0)
                        {
                            WiFi.disconnect();
                            vTaskDelay(500);
                            WiFi.begin(device.WiFi.Credentials[indexWiFiCredentials].SSID, device.WiFi.Credentials[indexWiFiCredentials].Password);
                            apIndex = apCount;
                            indexWiFiCredentials = 5;
                        }
                    }
                } 
                
                vTaskDelay(5000);
                Serial.printf("Reconnection to WiFi | Status Code: %d\n",WiFi.status());
            }
            deviceTimerStop();
            xEventGroupClearBits(ledStatusEvent,LED_STATUS_EVENT_WIFI_CONNECTING);
            status.ledOn();
            Serial.printf("\r\n\r\n-=WiFi Connection=-\r\n>SSID: %s\r\n>Status: %s | Code: %d\r\n",
                            WiFi.SSID().c_str(),
                            (WiFi.status() == WL_CONNECTED ? "Connected" : "Other"), WiFi.status());  
            Serial.printf(">IP Address Local: %s | Broadcast: %s\r\n\r\n"
                            ,WiFi.localIP().toString().c_str(), WiFi.broadcastIP().toString().c_str());
            if(device.mqtt.state == true)
            {
                vTaskResume(deviceMqttConnectionTaskHandle);
            }
        } 
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void device_configuration_Task(void *p)
{
    Serial.printf("\r\n\t\t\t\t\tDevice Coniguration Task Start.\r\n\r\n");
    vSemaphoreCreateBinary(configurationMutex);
    xSemaphoreTake(configurationMutex, 0);
    while (pdTRUE)
    {
        if (xSemaphoreTake(configurationMutex, portMAX_DELAY))
        {
            Serial.println("configurationTask xSemaphoreTake.");
            dataPrint(dataSegment);
            sortInformation(&device, dataSegment);
            setDeviceInformation(&device);
            printInformation(&device);
            status.ledOff();
            Serial.print("\r\n\r\nSystem Restarting");
            for (uint8_t index = 0; index < 5; index++)
            {
                Serial.print(".");
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            Serial.println("\r\n\r\n\r\n");
            ESP.restart();
        } 
    }
    vTaskDelete(NULL);
}

void device_led_status_task(void *p)
{
    Serial.printf("\r\n\t\t\t\t\tDevice LED Status Task Start.\r\n\r\n");
    uint8_t     ledState = 0x00;
    uint8_t     ledStateLock = 0x00;
    uint32_t    wifiPrevTime = 0x00; 
    while(pdTRUE)
    {
        uint32_t ledStatusEventState = xEventGroupWaitBits(ledStatusEvent,LED_STATUS_EVENT, false, false, 1); 
        uint32_t currentTime = millis(); 
        if((ledStatusEventState & LED_STATUS_EVENT_WIFI_CONNECTING) && ((ledStateLock & LED_STATUS_EVENT_WIFI_CONNECTING) || !(ledStateLock & LED_STATUS_EVENT_WIFI_CONNECTING)))
        { 
            ledStateLock = LED_STATUS_EVENT_WIFI_CONNECTING;
            if(!(ledState & LED_STATUS_EVENT_WIFI_CONNECTING) && ((currentTime - wifiPrevTime) >= LED_STATUS_WIFI_OFF_TIME))
            { 
                wifiPrevTime = currentTime;
                status.ledOn();
                ledState |= LED_STATUS_EVENT_WIFI_CONNECTING;
            }
            else if((ledState & LED_STATUS_EVENT_WIFI_CONNECTING) && ((currentTime - wifiPrevTime) >= LED_STATUS_WIFI_ON_TIME))
            { 
                wifiPrevTime = currentTime;
                status.ledOff();
                ledState &= ~LED_STATUS_EVENT_WIFI_CONNECTING;
            } 
        }
        else if((ledStatusEventState & LED_STATUS_EVENT_MQTT_CONNECTING) && ((ledStateLock & LED_STATUS_EVENT_MQTT_CONNECTING) || !(ledStateLock & LED_STATUS_EVENT_WIFI_CONNECTING)))
        { 
            ledStateLock = LED_STATUS_EVENT_MQTT_CONNECTING;
            if(!(ledState & LED_STATUS_EVENT_MQTT_CONNECTING) && ((currentTime - wifiPrevTime) >= LED_STATUS_MQTT_OFF_TIME))
            { 
                wifiPrevTime = currentTime;
                status.ledOn();
                ledState |= LED_STATUS_EVENT_MQTT_CONNECTING;
            }
            else if((ledState & LED_STATUS_EVENT_MQTT_CONNECTING) && ((currentTime - wifiPrevTime) >= LED_STATUS_MQTT_ON_TIME))
            { 
                wifiPrevTime = currentTime;
                status.ledOff();
                ledState &= ~LED_STATUS_EVENT_MQTT_CONNECTING;
            } 
        }
        else
        {
            ledStateLock = 0x00;
        }
        vTaskDelay(1/portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void device_serial_task(void *p)
{
    Serial.printf("\r\n\t\t\t\t\tDevice Serial Task Start.\r\n\r\n");
    char serialData[1024];
    uint32_t serialIndex = 0;
    uint8_t serialLock = pdTRUE;

    memset(serialData,'\0',sizeof(serialData));
    while(pdTRUE)
    {
        while(Serial.available())
        {
            serialData[serialIndex] = Serial.read();
            
            if(serialData[serialIndex] == '\r')
            {
                serialData[serialIndex] = 0x00;
                serialIndex = 0;
                memset(dataSegment, '\0', sizeof(dataSegment));
                dataProcess(serialData, dataSegment);

                if(serialLock == pdTRUE)
                {
                    if (!strcmp(dataSegment[0], "WRB+ACCESS=adminWRB"))
                    {
                        serialLock = pdFALSE;
                        Serial.printf("\n\nSerial unlock.\n\n");
                    }
                    else
                    {
                        Serial.printf("\n\nLocked Serial Terminal.\n\n");
                    }
                } 
                else if(serialLock == pdFALSE)
                {
                    if(!strcmp(dataSegment[0],"WRB+CP"))
                    {
                        printInformation(&device);
                    }
                    else if(!strcmp(dataSegment[0],"WRB+CONFIG"))
                    { 
                        dataPrint(dataSegment);
                        xSemaphoreGive(configurationMutex);
                    }
                    else if(!strcmp(dataSegment[0],"WRB+AP"))
                    {
                        Serial.printf("\nSoft AP Credentials:\nSSID: %s\nPASS: %s\n\n", device.WiFi.AP.SSID, device.WiFi.AP.Password);
                    }  
                    else
                    {
                        Serial.printf("\n\nInvalid command.\n\n");
                    }
                }
            }
            else
            {
                serialIndex++;
            }
        }
        vTaskDelay( 1 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void device_event_group(void)
{
    /* Event Group Create */
    relayEvent = xEventGroupCreate(); 
    ledStatusEvent = xEventGroupCreate();
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
    xTaskCreatePinnedToCore(device_relay_data_process_task,     "DeviceRelayDataProcess",       2024, NULL, DEVICE_TASKPRIORITIES_RELAY_DATA_PROCESS,       &deviceRelayDataProcessTaskHandle,  APP_CPU_NUM);
    xTaskCreatePinnedToCore(device_relay_green_activate_task,   "DeviceRelayGreenActivate",     2024, NULL, DEVICE_TASKPRIORITIES_RELAY_GREEN_ACTIVATE,     &deviceRelayGreenTaskHandle,        APP_CPU_NUM);
    xTaskCreatePinnedToCore(device_relay_orange_activate_task,  "DeviceRelayOrangeActivate",    2024, NULL, DEVICE_TASKPRIORITIES_RELAY_ORANGE_ACTIVATE,    &deviceRelayOrangeTaskHandle,       APP_CPU_NUM);
    xTaskCreatePinnedToCore(device_relay_red_activate_task,     "DeviceRelayRedActivate",       2024, NULL, DEVICE_TASKPRIORITIES_RELAY_RED_ACTIVATE,       &deviceRelayRedTaskHandle,          APP_CPU_NUM);
    xTaskCreatePinnedToCore(device_relay_cancel_activate_task,  "DeviceRelayRedActivate",       2024, NULL, DEVICE_TASKPRIORITIES_RELAY_CANCEL_ACTIVATE,    &deviceRelayCancelTaskHandle,       APP_CPU_NUM);
}
#endif


 
void device_init_task(void)
{
    Serial.begin(115200);
    Serial.printf("\r\n\t\t\t\t\tDevice Task Init Start.\r\n\r\n");
    Serial.printf("\r\n\t\t\t\t\tFW: %1.1fv\r\n\r\n",FW_VERSION);
    device_check_database(); 

    #ifdef  DEVICE_NORMAL_OPERATION
    device_event_group();
    device_queue();
    device_semaphore();
    #endif

    relay.begin( device.relay.green.onState, device.relay.orange.onState, device.relay.red.onState, device.relay.cancel.onState,
                 device.relay.green.offTime, device.relay.orange.offTime, device.relay.red.offTime, device.relay.cancel.offTime);   

    #ifdef  DEVICE_NORMAL_OPERATION
    #ifndef WEBSERVER_OFF
      web_server();
    #endif
    
    xTaskCreatePinnedToCore(device_configuration_Task, "DeviceConfiguration", 8192, NULL, DEVICE_TASKPRIORITIES_CONFIGURATION, &deviceConfigurationTaskHandle, APP_CPU_NUM); 
    xTaskCreatePinnedToCore(device_serial_task, "DeviceSerial", 8192,NULL,DEVICE_TASKPRIORITIES_SERIAL, &deviceSerialTaskHandle, APP_CPU_NUM);
    if(strcmp("c00000",device.ID.company))
    {  
        deviceTimerStart(120000 / portTICK_PERIOD_MS, pdFALSE);
        
        /* wait timer to be triggered or expire */
        while(!deviceTimerState()){ vTaskDelay(10000 / portTICK_PERIOD_MS);}
        xTaskCreatePinnedToCore(device_led_status_task, "DeviceLedStatus", 4096, NULL, DEVICE_TASKPRIORITIES_LED_STATUS, &deviceLedStatusTaskHandle, APP_CPU_NUM);
        xTaskCreatePinnedToCore(device_wifi_connection_task, "DeviceWiFi", 8192, NULL, DEVICE_TASKPRIORITIES_WIFI, &deviceWifiConnectionTaskHandle, APP_CPU_NUM);
        
        while(WiFi.status() != WL_CONNECTED)
        { 
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        xTaskCreatePinnedToCore(device_mqtt_connection_task, "DeviceMQTT", 8192, NULL, DEVICE_TASKPRIORITIES_MQTT, &deviceMqttConnectionTaskHandle, APP_CPU_NUM);
        xTaskCreatePinnedToCore(device_button_task, "DeviceButton", 4096, NULL, DEVICE_TASKPRIORITIES_BUTTON, &deviceButtonTaskHandle, APP_CPU_NUM);
        
        device_sub_task();  
    }  
    #endif
}   


