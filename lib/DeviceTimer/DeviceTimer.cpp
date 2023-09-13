#include <DeviceTimer.h>
#include <Status.h> 
extern Status status;
/* Timer's ID */
#define DEVICETIMER_ID_RESET        0x00
#define DEVICETIMER_ID_STATUS_LED   0x01
#define DEVICETIMER_ID_STATUS_LED   0x01

// You'll likely need this on vanilla FreeRTOS
//#include <timers.h>
typedef struct 
{
  uint8_t trigger;
  uint8_t reset;
}DeviceTimerType_t;

DeviceTimerType_t deviceTimer;
   

static uint8_t statusLedState = 0x00;

static TimerHandle_t one_shot_timer = NULL;
static TimerHandle_t status_timer = NULL;

void sleepTimerCallback(TimerHandle_t xTimer)
{
  if((uint32_t)pvTimerGetTimerID(xTimer) == DEVICETIMER_ID_RESET)
  {
    Serial.println("[DeviceTimer CB] Timer Expire"); 
    if(deviceTimer.reset == pdTRUE)
    {
      Serial.println("[DeviceTimer CB] Restarting."); 
      ESP.restart();
    } 
    deviceTimer.trigger = pdTRUE;
  }
}

void deviceTimerStart(TickType_t t, uint8_t reset)
{
  one_shot_timer = xTimerCreate(
                      "SleeTimer",  // Name of timer
                      t,            // Period of timer (in ticks)
                      pdFALSE,
                      (void *)DEVICETIMER_ID_RESET,    // Timer ID
                      sleepTimerCallback);
  if(one_shot_timer == NULL)
  { 
    Serial.println("[DeviceTimer event] Could not create timers");  
  }
  else
  {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.println("[DeviceTimer event] Start Timer.");  
    deviceTimer.trigger = pdFALSE;
    deviceTimer.reset = reset;
    xTimerStart(one_shot_timer, portMAX_DELAY); 
  }
}


void statusLedCallback(TimerHandle_t xTimer)
{  
  if((uint32_t)pvTimerGetTimerID(xTimer) == DEVICETIMER_ID_STATUS_LED)
  {
    status.ledToggle(); 
  }
}
 
void deviceStartStatusLed(TickType_t t)
{
  if(statusLedState == pdFALSE)
  {
    status_timer = xTimerCreate(
                      "StatusLED",  // Name of timer
                      t,            // Period of timer (in ticks)
                      pdTRUE,
                      (void *)DEVICETIMER_ID_STATUS_LED,    // Timer ID
                      statusLedCallback);
    if(status_timer == NULL)
    {
      Serial.println("[Status event] Could not create timer.");  
    } 
    else
    {
      Serial.println("[Status event] Start Timer.");  
      xTimerStart(status_timer, portMAX_DELAY);
      statusLedState = pdTRUE;
    }
  }
  else
  {
    Serial.println("[Status event] Timer is already activate"); 
  } 
}

void deviceStopStatusLed(void)
{
  if(statusLedState == pdTRUE)
  {
    Serial.println("[Status event] Stop Timer."); 
    xTimerDelete(status_timer,portMAX_DELAY);
    statusLedState = pdFALSE;
  }
  status.ledOn();
}

void deviceTimerStop(void)
{
  Serial.println("[DeviceTimer event] Stop Timer."); 
  xTimerDelete(one_shot_timer,portMAX_DELAY);
}

uint8_t deviceTimerState(void)
{
  return(deviceTimer.trigger);
}