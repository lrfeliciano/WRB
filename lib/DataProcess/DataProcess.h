#ifndef DATAPROCESS_h
#define DATAPROCESS_h

#include <Arduino.h>
#include <DeviceDatabase.h>



#define COMMAND_SET_MAX 39




/* Device Information Memory Map Max Character */
#define WATCH_ID_MAX                        17
#define MQTT_SERVER_IP_MAX                  40        
#define COMPANY_ID_MAX                      9
#define LOCATION_ID_MAX                     9         
#define WIFI_SSID_MAX                       27
#define WIFI_PASS_MAX                       27
#define PRIMARY_DNS_MAX                     16
#define SECONDARY_DNS_MAX                   16
#define STATIC_IP_MAX                       16
#define STATIC_IP_STATE_MAX                 2
#define SUBNET_MAX                          16
#define GATEWAY_MAX                         16
#define LON_MAX                             17
#define FALL_ENABLE_DURESS_LEVEL_MAX        2
#define FALL_ENABLE_DURESS_GROUP_MAX        2
#define FALL_ENABLE_STATE_MAX               2
#define BATTERY_LEVEL_MAX                   5
#define MOBILE_OR_FIXED_MAX                 2
#define MODE_MAX                            2
#define GREEN_GROUP_CODE_MAX                11
#define ORANGE_GROUP_CODE_MAX               11
#define RED_GROUP_CODE_MAX                  11
#define CANCEL_GROUP_CODE_MAX               3
#define GREEN_AUTO_OFF_TIME_MAX             4
#define ORANGE_AUTO_OFF_TIME_MAX            4
#define RED_AUTO_OFF_TIME_MAX               4
#define CANCEL_AUTO_OFF_TIME_MAX            4
#define RELAY_ON_STATE_MAX                  5
#define MQTT_SERVER_PORT_MAX                9 

extern DynamicJsonDocument deviceDatabase;


typedef struct
{
  char SSID[32];
  char Password[32];
}WiFiAttributeType_t;

typedef struct 
{
    char level;
    char group;
    char state;
}DuressType_t;

typedef struct
{
    char First[21];
    char Last[21]; 
}NameType_t;

typedef struct 
{
    char watch[17];
    char company[9];
    char location[9];
    char staff[9];
}IDType_t;

typedef struct
{
    char ADC[5];
    char Level[5];
}BatteryType_t;

typedef struct
{
    char topic[64];
    char payload[128];
}MQTTTopicType_t;

typedef struct
{
    MQTTTopicType_t hbMSQ;
    MQTTTopicType_t heartbeat;
    MQTTTopicType_t startUp;
    MQTTTopicType_t green;
    MQTTTopicType_t orange;
    MQTTTopicType_t red;
    MQTTTopicType_t cancel; 
    MQTTTopicType_t firmwareUpdateByID;
    MQTTTopicType_t firmwareUpdateByLID;
    MQTTTopicType_t firmwareUpdateByCID;
}MQTTEventTopicType_t;



typedef struct
{ 
    MQTTTopicType_t commands;
    MQTTTopicType_t configuration;
    MQTTTopicType_t localButton;
    MQTTEventTopicType_t event;

    char group[10];
    char alert[10];
    char IP[41];
    char port[9];
    uint8_t state;
}MQTTType_t;



typedef struct
{
    uint8_t     onState; // uint8_t
    uint32_t    offTime;      //uint32_t
    char        group[11];
}RelayConfigType_t;

typedef struct
{
    MQTTType_t mqtt;
    RelayConfigType_t green;
    RelayConfigType_t orange;
    RelayConfigType_t red;
    RelayConfigType_t cancel;
}WiFiRelayBoardType_T;

typedef struct
{
    char IP[16]; // IP
    char subnet[16];
    char gateway[16];
    char pDNS[16];
    char sDNS[16];
    char state[2];
}IPType_T;

typedef struct 
{
    uint8_t pin;
    uint8_t state;
}ButtonType_T; 

typedef struct
{
    WiFiAttributeType_t Credentials[6];
    WiFiAttributeType_t AP; 
}WiFiType_t;

typedef struct
{
    uint8_t active : 1;
    unsigned long time; // Time of the delay
    unsigned long duration; // Duration of the event will occur
}TimeType_t;

typedef struct 
{
    char mof; 
    char mode;
    char IPAddress[49];
    char LAT[17];
    char LON[17];  
    IPType_T _static; // Device Static IP
    uint32_t counter;
    String MAC; 
    // String SSID; 
    // String Password;
    BatteryType_t Battery;
    IDType_t ID; // Device ID list (ID,Company,Location)
    WiFiType_t WiFi; // Device wifi credentials 
    DuressType_t duress; 
    DuressType_t fall;
    // NameType_t Name;
    MQTTType_t mqtt; // MQTT topics and payloads
    WiFiRelayBoardType_T relay; // Relay Configuration
    // ButtonType_T green;
    // ButtonType_T orange;
    // ButtonType_T red;
    // ButtonType_T cancel;
    TimeType_t hb; // hearbeat delay publish
}DeviceType_t;




extern int strcnt(char *s);

/* remove a specific character from a string*/
void strrmchr(char *s, int c);

/* replace specific character from a string */
void strrpchr(char *s, char c, char n);

extern char *commandCheck(char*s, char *w);

extern void commmandGetVal(char *s,const char *w, char *d,uint8_t e);

void dataPrint(char (*d)[49]);

void dataProcess(char *s, char (*d)[49]);
 
void printInformation(DeviceType_t *info);
void sortInformation(DeviceType_t *info, char (*d)[49]);

/* SAVE CONFIGURATION FUNCTIONS*/

void databaseInit(void);


void clearDeviceInformation(void);
void getDeviceInformation(DeviceType_t *info);
void printDeviceInformation(DeviceType_t *info);
void setDeviceInformation(DeviceType_t *info);

#endif