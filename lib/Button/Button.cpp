#include <Button.h>


const byte interruptPin = 34;

volatile int interruptCounter[4] = {0,0,0,0};  

portMUX_TYPE muxs = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR button1Interrupt() 
{
  portENTER_CRITICAL_ISR(&muxs);
  if(interruptCounter[0] == 0)
  {
      interruptCounter[0]++;
  } 
  portEXIT_CRITICAL_ISR(&muxs);
}

void IRAM_ATTR button2Interrupt() 
{
  portENTER_CRITICAL_ISR(&muxs);
  if(interruptCounter[1] == 0)
  {
      interruptCounter[1]++;
  } 
  portEXIT_CRITICAL_ISR(&muxs);
}

void IRAM_ATTR button3Interrupt() 
{
  portENTER_CRITICAL_ISR(&muxs);
  if(interruptCounter[2] == 0)
  {
      interruptCounter[2]++;
  } 
  portEXIT_CRITICAL_ISR(&muxs);
}

void IRAM_ATTR button4Interrupt() 
{
  portENTER_CRITICAL_ISR(&muxs);
  if(interruptCounter[3] == 0)
  {
      interruptCounter[3]++;
  } 
  portEXIT_CRITICAL_ISR(&muxs);
}


Button::Button(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4):
    button_1_pin(b1), button_2_pin(b2), button_3_pin(b3), button_4_pin(b4)
{

}

void Button::begin(void)
{
    /* Set button pin as input mode */
    pinMode(button_1_pin, INPUT);
    pinMode(button_2_pin, INPUT);
    pinMode(button_3_pin, INPUT);
    pinMode(button_4_pin, INPUT);

    /* Attach interrupt pin and mode */
    attachInterrupt(digitalPinToInterrupt(button_1_pin), button1Interrupt, RISING);
    attachInterrupt(digitalPinToInterrupt(button_2_pin), button2Interrupt, RISING);
    attachInterrupt(digitalPinToInterrupt(button_3_pin), button3Interrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(button_4_pin), button4Interrupt, FALLING); // RISING);
}

void Button::ButtonFunction1Address(void (*f)())
{
    butFunc1 = f;
}

void Button::ButtonFunction1Process(void)
{
  if(interruptCounter[0] > 0)
  { 
      portENTER_CRITICAL(&muxs);
      interruptCounter[0]--;
      portEXIT_CRITICAL(&muxs); 
      butFunc1(); // execute the button 1 function
  } 
}

void Button::ButtonFunction2Address(void (*f)())
{
    butFunc2 = f;
}

void Button::ButtonFunction2Process(void)
{

  if(interruptCounter[1] > 0)
  { 
      portENTER_CRITICAL(&muxs);
      interruptCounter[1]--;
      portEXIT_CRITICAL(&muxs); 
      
      butFunc2(); // execute the button 1 function
  } 
}

void Button::ButtonFunction3Address(void (*f)())
{
    butFunc3 = f;
}

void Button::ButtonFunction3Process(void)
{
  if(interruptCounter[2] > 0)
  { 
      portENTER_CRITICAL(&muxs);
      interruptCounter[2]--;
      portEXIT_CRITICAL(&muxs); 
      butFunc3(); // execute the button 1 function
  } 
}

void Button::ButtonFunction4Address(void (*f)())
{
    butFunc4 = f;
}

void Button::ButtonFunction4Process(void)
{
  if(interruptCounter[3] > 0)
  { 
      portENTER_CRITICAL(&muxs);
      interruptCounter[3]--;
      portEXIT_CRITICAL(&muxs); 
      butFunc4(); // execute the button 1 function
  } 
}

uint8_t Button::Button1Read(void)
{ 
  return(digitalRead(button_1_pin));
}

uint8_t Button::Button2Read(void)
{ 
  return(digitalRead(button_2_pin));
}

uint8_t Button::Button3Read(void)
{ 
  return(digitalRead(button_3_pin));
}

uint8_t Button::Button4Read(void)
{ 
  return(digitalRead(button_4_pin));
}

void Button::Button2InvertState(void)
{
  detachInterrupt(button_2_pin);
  vTaskDelay(100);
  if(button_2_state)
  {
    attachInterrupt(digitalPinToInterrupt(button_2_pin), button2Interrupt, FALLING);
  }
  else
  {
    attachInterrupt(digitalPinToInterrupt(button_2_pin), button2Interrupt, RISING);
  }
  button_2_state = !button_2_state;
}

uint8_t Button::Button2State(void)
{
  return(button_2_state);
}

void Button::Button3InvertState(void)
{
  detachInterrupt(button_3_pin);
  vTaskDelay(100);
  if(button_3_state)
  {
    attachInterrupt(digitalPinToInterrupt(button_3_pin), button3Interrupt, FALLING);
  }
  else
  {
    attachInterrupt(digitalPinToInterrupt(button_3_pin), button3Interrupt, RISING);
  }
  button_3_state = !button_3_state;
}

uint8_t Button::Button3State(void)
{
  return(button_3_state);
}

uint8_t Button::Button4State(void)
{
  return(button_4_state);
}