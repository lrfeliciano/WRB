
#include <DataProcess.h>
#include <EEPROM.h>
#include <string.h>

const char *accronym[]=
{
    "cmd",
    "co_idn",
    "lo_idn",
    "wa_idn",
    "fi_nam",
    "la_nam",
    "st_idn",
    "wa_lat",
    "wa_lon",
    "wa_bat",
    "gr_but",
    "du_lev",
    "wa_mof",
    "cd_con",
    "cc_ack"
}; 
const uint8_t commandElement[] = 
{
    0,                      // 1. COMMAND
    0,                      // 2. CC_ACK
    WATCH_ID_MAX,           // 3. Watch ID
    MQTT_SERVER_IP_MAX,     // 4. MQTT Server IP address
    COMPANY_ID_MAX,         // 5. Company ID
    LOCATION_ID_MAX,        // 6. Location ID
    WIFI_SSID_MAX,          // 7. SSID 0
    WIFI_SSID_MAX,          // 8. SSID 1
    WIFI_SSID_MAX,          // 9. SSID 2
    WIFI_SSID_MAX,          // 10. SSID 3 
    WIFI_SSID_MAX,          // 11. SSID 4
    WIFI_PASS_MAX,          // 12. Password 0
    WIFI_PASS_MAX,          // 13. Password 1 
    WIFI_PASS_MAX,          // 14. Password 2 
    WIFI_PASS_MAX,          // 15. Password 3 
    WIFI_PASS_MAX,          // 16. Password 4
    PRIMARY_DNS_MAX,        // 17. Primary DNS
    SECONDARY_DNS_MAX,      // 18. Secondary DNS
    STATIC_IP_MAX,          // 19. Static IP
    STATIC_IP_STATE_MAX,    // 20. Static IP State
    SUBNET_MAX,             // 21. Subnet Mask
    GATEWAY_MAX,            // 22. Default Gateway
    LON_MAX,                // 23. LON
    FALL_ENABLE_DURESS_LEVEL_MAX,      // 24. Fall Enable Duress Level
    FALL_ENABLE_DURESS_GROUP_MAX,      // 25. Fall Enable Duress Group
    FALL_ENABLE_STATE_MAX,      // 26. Fall Enable State
    BATTERY_LEVEL_MAX,      // 27. Battery level
    MOBILE_OR_FIXED_MAX,      // 28. MOBILE OR FIXED
    MODE_MAX,      // 29. MODE
    GREEN_GROUP_CODE_MAX,     // 30. Green Group Code
    ORANGE_GROUP_CODE_MAX,     // 31. Orange Group Code
    RED_GROUP_CODE_MAX,     // 32. Red Group Code
    CANCEL_GROUP_CODE_MAX,      // 33. Cancel Group Code
    GREEN_AUTO_OFF_TIME_MAX,      // 34. Green Auto Off Time
    ORANGE_AUTO_OFF_TIME_MAX,      // 35. Orange Auto Off Time
    RED_AUTO_OFF_TIME_MAX,      // 36. Red Auto Off Time
    CANCEL_AUTO_OFF_TIME_MAX,      // 37. Cancel Auto Off Time
    RELAY_ON_STATE_MAX,      // 38. Relay On state
    MQTT_SERVER_PORT_MAX       // 39. MQTT Server Port 
};

const char *commandSet[]=
{
    "cmd",          // 1. COMMAND
    "cc_ack",       // 2. CC_ACK
    "wa_idn",       // 3. Watch ID
    "mqttip",       // 4. Mqtt Server IP address
    "co_idn",       // 5. Company ID
    "lo_idn",       // 6. Location ID
    "ssid_0",       // 7. SSID 0
    "ssid_1",       // 8. SSID 1
    "ssid_2",       // 9. SSID 2
    "ssid_3",       // 10. SSID 3 
    "ssid_4",       // 11. SSID 4
    "pass_0",       // 12. Password 0
    "pass_1",       // 13. Password 1 
    "pass_2",       // 14. Password 2
    "pass_3",       // 15. Password 3
    "pass_4",       // 16. Password 4
    "p_dns",        // 17. Primary DNS
    "s_dns",        // 18. Secondary DNS
    "sta_ip",       // 19. Static IP
    "staips",       // 20. Static IP State
    "subnet",       // 21. Subnet Mask
    "gate_w",       // 22. Default Gateway
    "fx_lon",       // 23. LON
    "fall_l",       // 24. Fall Enable Duress Level
    "fall_g",       // 25. Fall Enable Duress Group
    "fall_e",       // 26. Fall Enable State
    "wa_bat",       // 27. Battery level
    "wa_mof",       // 28. MOBILE OR FIXED
    "b_mode",       // 29. MODE
    "grn_gc",       // 30. Green Group Code
    "org_gc",       // 31. Orange Group Code
    "red_gc",       // 32. Red Group Code
    "can_gc",       // 33. Cancel Group Code
    "grn_ot",       // 34. Green Auto Off Time
    "org_ot",       // 35. Orange Auto Off Time
    "red_ot",       // 36. Red Auto Off Time
    "can_ot",       // 37. Cancel Auto Off Time
    "rly_os",       // 38. Relay On state
    "mqtt_p"       // 39. Mqtt Server Port   

};

int strcnt(const char *s)
{
    char *c = (char *)s;
    while(*s != '\0') s++;
    return((s-c)+1);
}
/* remove a specific character from a string*/
void strrmchr(char *s, int c){ 
  
    int j, n = strlen(s); 
    for (int i=j=0; i<n; i++) 
       if (s[i] != c) 
          s[j++] = s[i]; 
      
    s[j] = '\0'; 
} 

/* replace specific character from a string */
void strrpchr(char *s, char c, char n)
{
    while(*s != '\0')
    {
        // printf("%c\r\n",*s);
        if(*s == c)
        {
            *s= n;
        }
        s++;
    }
}

char *commandCheck(char*s, const char *w)
{
    char *sp, *ep;
    sp = strstr(s,w);
    if(sp)
    {
        ep = strchr(sp,'=');
        if((ep-sp)==strlen(w)) return sp;
    }
    return 0;
}

/**
	* @brief  Check Schedule 
	* @param  s1		:		schedule structure
	* @param  s2		:		schedule structure
	* @param  s3		:		schedule structure
	* @retval None
	*/

void commmandGetVal(char *s,const char *w, char *d,uint8_t e)
{
    char *sp;
    uint8_t l;
    sp = commandCheck(s,w); 
    if(sp)
    {
        sp += strcnt(w); 
        l = strchr(sp,'?') - sp;
        if((l >= e) && (e != 0)) 
            l = (e-1);
        (l) ? strncpy(d,sp,l) : strcpy(d,"*");
    }
    else
    {
        strcpy(d,"*");
    }
    
}

void dataPrint(char (*d)[49])
{ 
    Serial.println("\r\n");
    Serial.println("-----------------Data Print-------------------"); 
    for(int i = 0; i < COMMAND_SET_MAX ;i++)
    {
        Serial.print("Command: "); Serial.print(commandSet[i]); 
        Serial.print("\t|\tDataSegment["); Serial.print(i);
        Serial.print("]: ");Serial.println(d[i]);
    }
    Serial.println("----------------------------------------------\r\n"); 
}

void dataProcess(char *s, char (*d)[49])
{    
    Serial.println("\r\n");
    Serial.println("----------------Data Process------------------");  
    
    for(uint8_t i = 0; i < COMMAND_SET_MAX ;i++)
    {                    
        commmandGetVal(s,commandSet[i],d[i],commandElement[i]); 
    } 
    Serial.println("----------------------------------------------\r\n"); 
}

void sortInformation(DeviceType_t *info, char (*d)[49])
{ 
    Serial.println("\r\n");
    Serial.println("--------------Sort Information----------------");  
    
    if (strcmp(d[2], "*"))
    {
        strcpy(info->ID.watch, d[2]);
    }

    if (strcmp(d[3], "*"))
    {
        strcpy(info->mqtt.IP, d[3]);
    }

    if (strcmp(d[4], "*"))
    {
        strcpy(info->ID.company, d[4]);
    }

    if (strcmp(d[5], "*"))
    {
        // printf("idlocation-d[5]: %c",d[5]); 
        strcpy(info->ID.location, d[5]);
    }

    for (int index = 0; index < 5; index++)
    {

        if (strcmp(d[index + 6], "*"))
        {
            strcpy(info->WiFi.Credentials[index].SSID, d[index + 6]);
        }

        if (strcmp(d[index + 11], "*"))
        {
            strcpy(info->WiFi.Credentials[index].Password, d[index + 11]);
        }
    }

    if (strcmp(d[16], "*"))
    {
        strcpy(info->_static.pDNS, d[16]); 
    }

    if (strcmp(d[17], "*"))
    {
        strcpy(info->_static.sDNS, d[17]);  
    }

    if (strcmp(d[18], "*"))
    { 
        strcpy(info->_static.IP, d[18]); 
    }

    if (strcmp(d[19], "*"))
    {
        // strcpy(info->Name.Last, d[19]);
        strcpy(info->_static.state, d[19]);
        // info->_static.state = d[19][0];
    }

    if (strcmp(d[20], "*"))
    {
        strcpy(info->_static.subnet, d[20]);
    }

    if (strcmp(d[21], "*"))
    {
        strcpy(info->_static.gateway, d[21]);
    }

    if (strcmp(d[22], "*"))
    {
        strcpy(info->LON, d[22]);
    }

    if (d[23][0] != '*')
    {
        info->fall.level = d[23][0];
    }

    if (d[24][0] != '*')
    {
        info->fall.group = d[24][0];
    }

    if (d[25][0] != '*')
    {
        info->fall.state = d[25][0];
    }

    if (strcmp(d[26], "*"))
    {
        strcpy(info->Battery.Level, d[26]);
    }

    if (d[27][0] != '*')
    {
        info->mof = d[27][0];
    }

    if (d[28][0] != '*')
    {
        info->mode = d[28][0];
    }
    
    if (strcmp(d[29], "*"))
    {
        strrmchr(d[29],',');
        strcpy(info->relay.green.group, d[29]);
    }
    if (strcmp(d[30], "*"))
    {
        strrmchr(d[30],',');
        strcpy(info->relay.orange.group, d[30]);
    }
    if (strcmp(d[31], "*"))
    {
        strrmchr(d[31],',');
        strcpy(info->relay.red.group, d[31]);
    }
    if (strcmp(d[32], "*"))
    {
        strrmchr(d[32],',');
        strcpy(info->relay.cancel.group, d[32]);
    }
    if (strcmp(d[33], "*"))
    {
        info->relay.green.offTime =  atoi(d[33]);
        if(info->relay.green.offTime > 255)
        {
            info->relay.green.offTime = 0xFF;
        }
    }
    if (strcmp(d[34], "*"))
    {
        info->relay.orange.offTime =  atoi(d[34]);
        if(info->relay.orange.offTime > 255)
        {
            info->relay.orange.offTime = 0xFF;
        }
    }
    if (strcmp(d[35], "*"))
    {
        info->relay.red.offTime =  atoi(d[35]);
        if(info->relay.red.offTime > 255)
        {
            info->relay.red.offTime = 0xFF;
        }
    }
    if (strcmp(d[36], "*"))
    {
        info->relay.cancel.offTime =  atoi(d[36]);
        if(info->relay.cancel.offTime > 255)
        {
            info->relay.cancel.offTime = 0xFF;
        }
        else if(info->relay.cancel.offTime == 0)
        {
            info->relay.cancel.offTime = 0x01;
        }
    }
    if (strcmp(d[37], "*"))
    {
        if (d[37][0] != '*')
        {
            info->relay.cancel.onState =  d[37][0] - 0x30;
        } 
        
        if (d[37][1] != '*')
        {
            info->relay.red.onState =  d[37][1] - 0x30;
        }
        if (d[37][2] != '*')
        {
            info->relay.orange.onState =  d[37][2] - 0x30;
        }
        if (d[37][3] != '*')
        {
            info->relay.green.onState =  d[37][3] - 0x30;
        }
    }
    if (strcmp(d[38], "*"))
    {
        strcpy(info->mqtt.port, d[38]);
    }

    

    Serial.println("----------------------------------------------\r\n"); 
}

void printInformation(DeviceType_t *info)
{ 
    Serial.printf("\r\n\r\n");
    Serial.printf("-----------------Print info-------------------\r\n");

    Serial.printf("\"DEVICE INFO\"\r\n"); 
    Serial.printf(">Watch ID: %s \r\n",info->ID.watch); 
    Serial.printf(">Company ID: %s \r\n",info->ID.company); 
    Serial.printf(">Location ID: %s \r\n\r\n",info->ID.location);

    Serial.printf(  "\"NETWORL DETAILS\"\r\n>State: %s\r\n>STATIC IP: %s\r\n"
                    ">SUBNET MASK: %s\r\n>Default Gateway: %s\r\n"
                    ">Primary DNS: %s\r\n>Secondary DNS: %s\r\n\r\n",
                    (info->_static.state[0] == '1' ? "Enabled" : "Disabled"),
                    info->_static.IP,
                    info->_static.subnet,
                    info->_static.gateway,
                    info->_static.pDNS,
                    info->_static.sDNS);  

    Serial.printf("\"MQTT SERVER DETAILS\"\r\n"); 
    Serial.printf(">IP: %s | Port: %s\r\n\r\n",info->mqtt.IP,info->mqtt.port);  

    
    Serial.printf("\"WIFI CREDENTIALS\"\r\n");
    for (uint8_t index = 0; index < 5; index++)
    {  
        Serial.printf(">SSID[%d]: %s | PASS[%d]: %s\r\n",index ,info->WiFi.Credentials[index].SSID ,index ,info->WiFi.Credentials[index].Password); 
    }
    
    Serial.printf("\r\n\"RELAY INFO\"\r\n");
    Serial.printf(">Group Code:\r\n->Green: %s\r\n->Orange: %s\r\n->Red: %s\r\n->Cancel: %s\r\n"
                    ,info->relay.green.group,info->relay.orange.group
                    ,info->relay.red.group,info->relay.cancel.group);

    Serial.printf("\r\n>Off Time:\r\n->Green: %u\r\n->Orange: %u\r\n->Red: %u\r\n->Cancel: %u\r\n"
                    ,info->relay.green.offTime ,info->relay.orange.offTime
                    ,info->relay.red.offTime ,info->relay.cancel.offTime);

    Serial.printf("\r\n>On State:\r\n->Green: %d\r\n->Orange: %d\r\n->Red: %d\r\n->Cancel: %d\r\n"
                    ,info->relay.green.onState,info->relay.orange.onState
                    ,info->relay.red.onState,info->relay.cancel.onState); 
    Serial.println("----------------------------------------------\r\n"); 
}


void clearDeviceInformation(void)
{
    Serial.println("\r\n");
    Serial.println("--------------CLEAR DEVICE INFO---------------");   
    deviceDatabaseRemove();  
    Serial.println("----------------------------------------------\r\n"); 
}

void getDeviceInformation(DeviceType_t *info)
{
    Serial.println("\r\n");
    Serial.println("--------------GET DEVICE INFO-----------------"); 
    deviceDatabaseOpen(); 
    strcpy(info->ID.watch, deviceDatabase["wa_idn"]);
    strcpy(info->_static.IP, deviceDatabase["sta_ip"]);
    strcpy(info->_static.state, deviceDatabase["staips"]);
    strcpy(info->_static.subnet, deviceDatabase["subnet"]); 
    strcpy(info->_static.gateway, deviceDatabase["gate_w"]); 
    strcpy(info->_static.pDNS, deviceDatabase["p_dns"]); 
    strcpy(info->_static.sDNS, deviceDatabase["s_dns"]); 
    strcpy(info->mqtt.IP, deviceDatabase["mqttip"]);
    strcpy(info->mqtt.port, deviceDatabase["mqtt_p"]);
    strcpy(info->ID.company, deviceDatabase["co_idn"]);
    strcpy(info->ID.location, deviceDatabase["lo_idn"]);
    strcpy(info->WiFi.Credentials[0].SSID, deviceDatabase["ssid_0"]);
    strcpy(info->WiFi.Credentials[1].SSID, deviceDatabase["ssid_1"]);
    strcpy(info->WiFi.Credentials[2].SSID, deviceDatabase["ssid_2"]);
    strcpy(info->WiFi.Credentials[3].SSID, deviceDatabase["ssid_3"]);
    strcpy(info->WiFi.Credentials[4].SSID, deviceDatabase["ssid_4"]);
    strcpy(info->WiFi.Credentials[0].Password, deviceDatabase["pass_0"]);
    strcpy(info->WiFi.Credentials[1].Password, deviceDatabase["pass_1"]);
    strcpy(info->WiFi.Credentials[2].Password, deviceDatabase["pass_2"]);
    strcpy(info->WiFi.Credentials[3].Password, deviceDatabase["pass_3"]);
    strcpy(info->WiFi.Credentials[4].Password, deviceDatabase["pass_4"]); 

    strcpy(info->relay.green.group, deviceDatabase["grn_gc"]);
    strcpy(info->relay.orange.group, deviceDatabase["org_gc"]);
    strcpy(info->relay.red.group, deviceDatabase["red_gc"]);
    strcpy(info->relay.cancel.group, deviceDatabase["can_gc"]);

    info->relay.green.offTime = deviceDatabase["grn_ot"];
    info->relay.orange.offTime = deviceDatabase["org_ot"];
    info->relay.red.offTime = deviceDatabase["red_ot"];
    info->relay.cancel.offTime = deviceDatabase["can_ot"];

    const char *relayState = deviceDatabase["rly_os"];

    info->relay.green.onState = relayState[0] - 0x30;
    info->relay.orange.onState = relayState[1] - 0x30;
    info->relay.red.onState = relayState[2] - 0x30;
    info->relay.cancel.onState = relayState[3] - 0x30;  
    Serial.println("----------------------------------------------\r\n"); 
} 

void setDeviceInformation(DeviceType_t *info)
{
    Serial.println("\r\n");
    Serial.println("--------------SET DEVICE INTO-----------------");  
    
    /* Device ID */
    deviceDatabase["wa_idn"] = info->ID.watch;
    deviceDatabase["co_idn"] = info->ID.company;
    deviceDatabase["lo_idn"] = info->ID.location;
    
    /* Network */
    deviceDatabase["sta_ip"] = info->_static.IP;
    deviceDatabase["staips"] = info->_static.state; 
    deviceDatabase["subnet"] = info->_static.subnet;
    deviceDatabase["gate_w"] = info->_static.gateway;
    deviceDatabase["p_dns"] = info->_static.pDNS;
    deviceDatabase["s_dns"] = info->_static.sDNS;

    /* MQTT Details */
    deviceDatabase["mqttip"] = info->mqtt.IP;
    deviceDatabase["mqtt_p"] = info->mqtt.port;

    /* WiFi Credentials */
    deviceDatabase["ssid_0"] = info->WiFi.Credentials[0].SSID;
    deviceDatabase["ssid_1"] = info->WiFi.Credentials[1].SSID;
    deviceDatabase["ssid_2"] = info->WiFi.Credentials[2].SSID;
    deviceDatabase["ssid_3"] = info->WiFi.Credentials[3].SSID;
    deviceDatabase["ssid_4"] = info->WiFi.Credentials[4].SSID;
    deviceDatabase["pass_0"] = info->WiFi.Credentials[0].Password;
    deviceDatabase["pass_1"] = info->WiFi.Credentials[1].Password;
    deviceDatabase["pass_2"] = info->WiFi.Credentials[2].Password;
    deviceDatabase["pass_3"] = info->WiFi.Credentials[3].Password;
    deviceDatabase["pass_4"] = info->WiFi.Credentials[4].Password; 

    /* Relay Group Code */
    deviceDatabase["grn_gc"] = info->relay.green.group;
    deviceDatabase["org_gc"] = info->relay.orange.group;
    deviceDatabase["red_gc"] = info->relay.red.group;
    deviceDatabase["can_gc"] = info->relay.cancel.group;

    /* Relay ON Time */
    deviceDatabase["grn_ot"] = info->relay.green.offTime;
    deviceDatabase["org_ot"] = info->relay.orange.offTime;
    deviceDatabase["red_ot"] = info->relay.red.offTime;
    deviceDatabase["can_ot"] = info->relay.cancel.offTime;

    /* Relay ON State */
    char relayState[5];
    sprintf(relayState,"%d%d%d%d", info->relay.green.onState, info->relay.orange.onState, info->relay.red.onState, info->relay.cancel.onState);
    deviceDatabase["rly_os"] = relayState;
    deviceDatabaseSave();
    Serial.println("----------------------------------------------\r\n"); 
}

void printDeviceInformation(DeviceType_t *info)
{ 
    Serial.print("Watch ID: ");
    Serial.println(info->ID.watch);
    Serial.print("IP: ");
    Serial.println(info->IPAddress);
    Serial.printf("STATIC IP: ");
    Serial.println(info->_static.IP);
    Serial.printf("Static IP State: %s\r\n",info->_static.state[0] == '1' ? "Enabled" : "Disabled");
    Serial.print("Company ID: ");
    Serial.println(info->ID.company);
    Serial.print("Location ID: ");
    Serial.println(info->ID.location);
    for (uint8_t index = 0; index < 5; index++)
    {
        Serial.print("WiFi SSID [");
        Serial.print(index);
        Serial.print("]: ");
        Serial.println(info->WiFi.Credentials[index].SSID);
        Serial.print("\t|\tWiFi PASS [");
        Serial.print(index);
        Serial.print("]: ");
        Serial.println(info->WiFi.Credentials[index].Password);
    }
    Serial.print("Duress Level: ");
    Serial.println(info->duress.level);
    Serial.print("Duress Group: ");
    Serial.println(info->duress.group); 
    Serial.print("Staff ID:");
    Serial.println(info->ID.staff);
    Serial.print("LAT: ");
    Serial.println(info->LAT);
    Serial.print("LON: ");
    Serial.println(info->LON);
    Serial.println("Fall Even State:");
    Serial.print("Level: ");
    Serial.println(info->fall.level);
    Serial.print("Group: ");
    Serial.println(info->fall.group);
    Serial.print("State: ");
    Serial.println(info->fall.state);
    Serial.print("Battery Level: ");
    Serial.println(info->Battery.Level);
}

void getWatchID(DeviceType_t *info)
{
    strcpy(info->ID.watch,deviceDatabase["wa_idn"]);
    Serial.printf("\r\nGet wid: %s\r\n",info->ID.watch);
}

void setWatchID(DeviceType_t *info)
{
    strcpy(info->ID.watch,deviceDatabase["wa_idn"]);
    Serial.printf("\r\nGet wid: %s\r\n",info->ID.watch);
    deviceDatabaseSave();
}

void databaseInit(void)
{
    deviceDatabaseInit();
} 