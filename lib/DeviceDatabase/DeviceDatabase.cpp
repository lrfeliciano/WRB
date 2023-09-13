
#include <DeviceDatabase.h>
#include "FS.h"
#include "SPIFFS.h"
 
          
extern DynamicJsonDocument deviceDatabase(2048);

#define FORMAT_SPIFFS_IF_FAILED true


#ifdef ENCRYPTED_DB
#include <Cryptography.h>
char eConfig[2048]; 
uint8_t dConfig[2048];

void deviceDatabaseCreate(void)
{
    printf("\r\n\r\n------------------Create DB-------------------\r\n\r\n");
    File file = SPIFFS.open(DEVICEDATABASE_ADDR,FILE_WRITE);
    file.print("XOtUUOOSjpaSES9SLUUmRUUUUU4UUOmO9SjpaSEOXUUmRUUmjmEmUULmjmSmvULmpULmUUUU4UUOmO9SjSEOXOmUUmRUUmXUUU4UUOmOpSUSLSpO9UUmRUUmUmpmpULmUmpmpULmUmpmpULmXUUU4UUSOSjO9SppaOOUUmRUUmjmEmUULmjmSmvULmpULmjUUU4UUOXpaS9SLOmUUmRUUmvULmvULmvULmvUUU4UUOmpaS9SLOmUUmRUUmvULmvULm9ULm9UUU4UUSeOjO9O9SEOXUUmRUUmpmEULmjmpm9ULmEmEULmjmSmpUUU4UUSeOjO9O9paOXUUmRUUmjmvmvm9UUU4UUSmSapaSES9SLUUmRUUSmmXmXmXmXmXUUU4UUS4SapaSES9SLUUmRUU94mXmXmXmXmjUUU4UUOmOmSES9pamXUUmRUUUUU4UUOmOmSES9pamjUUmRUUUUU4UUOmOmSES9pamUUUmRUUUUU4UUOmOmSES9pammUUmRUUUUU4UUOmOmSES9pam9UUmRUUUUU4UUOXSjOmOmpamXUUmRUUUUU4UUOXSjOmOmpamjUUmRUUUUU4UUOXSjOmOmpamUUUmRUUUUU4UUOXSjOmOmpammUUmRUUUUU4UUOXSjOmOmpam9UUmRUUUUU4UUSOOUSLpaSOSmUUmRUUmXUUU4UUSaOUSOpaSOSmUUmRUUmXUUU4UUOUSpS9paSOSmUUmRUUmXUUU4UUSmSjSLpaSOSmUUmRUU9t9jUUU4UUSOOUSLpaSaO9UUmRmjU4UUSaOUSOpaSaO9UUmRmjU4UUOUSpS9paSaO9UUmRmjU4UUSmSjSLpaSaO9UUmRmjU4UUOUS4OEpaSaOmUUmRUUmXmXmXmXUUOe");
    file.close();
    printf("\r\n\r\n----------------------------------------------\r\n\r\n");
}

uint8_t deviceFileCheck(void)
{
    File file = SPIFFS.open(DEVICEDATABASE_ADDR);
    if(!file || file.isDirectory())
    {
        return 0;
    }
    return 1;
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

uint8_t deviceDatabaseInit(void)
{
    uint8_t state = 0x00;
    printf("\r\n\r\n--------------------Init DB-------------------\r\n\r\n");
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
    {
        printf("\r\nSPIFFS Mount Failed\r\n");
        return 0;
    }
    
    if(!deviceFileCheck())
    {
        printf("\r\nDatabase not found.\r\n"); 
        deviceDatabaseCreate();
        state = 2;
    }
    listDir(SPIFFS, "/", 0);
    deviceDatabaseOpen();
    printf("\r\n\r\n----------------------------------------------\r\n\r\n");
    state |= 1;
    return state;
}

void deviceDatabaseOpen(void)
{
    memset(eConfig,'\0',sizeof(eConfig));
    File file = SPIFFS.open(DEVICEDATABASE_ADDR); 
    printf("\r\n\r\n--------------------Open DB-------------------\r\n\r\n");
    file.read((uint8_t *)eConfig,file.size()); 
    Serial.printf("%s\r\n",eConfig);
    decrypt((char*)eConfig,(char*)dConfig);
    deserializeJson(deviceDatabase, dConfig); 
    file.close();
    
    printf("\r\n\r\n----------------------------------------------\r\n\r\n");
}

void deviceDatabaseSave(void)
{
    encrypt((char*) deviceDatabase.as<String>().c_str(),eConfig); 
    File file = SPIFFS.open(DEVICEDATABASE_ADDR,FILE_WRITE);
    file.print(eConfig);
    file.close();

}

void deviceDatabaseRemove(void)
{
    if(SPIFFS.remove(DEVICEDATABASE_ADDR))
    {
        printf("\r\nDatabase remove successfully.\r\n");
        deviceDatabaseCreate();
    }
    else
    {
        printf("\r\nDatabase remove failed.\r\n");
    }  
}

void deviceDatabasePrint(void)
{
    printf("\r\n\r\n--------------------Print DB-------------------\r\n\r\n");
    serializeJson(deviceDatabase, Serial);
    printf("\r\n\r\n----------------------------------------------\r\n\r\n");
} 
#endif
