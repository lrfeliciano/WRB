#include <DeviceTimer.h>

/* Timer's ID */
#define DEVICETIMER_ID_RESET 0x00 
typedef struct 
{
  uint8_t trigger;
  uint8_t reset;
}DeviceTimerType_t;

DeviceTimerType_t deviceTimer;
 
// Settings
static const TickType_t configTimerTick = 120000 / portTICK_PERIOD_MS;
// Globals

static TimerHandle_t one_shot_timer = NULL;

void sleepTimerCallback(TimerHandle_t xTimer)
{
  if((uint32_t)pvTimerGetTimerID(xTimer) == DEVICETIMER_ID_RESET)
  {
    Serial.println("Timer Expire");
    if(deviceTimer.reset == pdTRUE)
    {
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
    Serial.println("Could not create timers"); 
  }
  else
  {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.println("Starting Sleep Timer"); 
    deviceTimer.trigger = pdFALSE;
    xTimerStart(one_shot_timer, portMAX_DELAY); 
  }
}

void deviceTimerStop(void)
{
  xTimerDelete(one_shot_timer,portMAX_DELAY);
}

uint8_t deviceTimerState(void)
{
  return(deviceTimer.trigger);
}