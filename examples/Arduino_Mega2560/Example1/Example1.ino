#include "nOS.h"

nOS_Timer timer;
nOS_Thread threadLow;
nOS_Thread threadHigh;
nOS_Sem semLow;
nOS_Sem semHigh;
nOS_Stack stackLow[128];
nOS_Stack stackHigh[128];

NOS_ISR(TIMER2_COMPA_vect)
{
  nOS_Tick();
  nOS_TimerTick();
  nOS_TimeTick();
}

static void Timer2Init(void)
{
  //set timer2 interrupt at 1kHz
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 1khz increments
  OCR2A = 249;// = (16*10^6) / (1000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS22 bit for 64 prescaler
  TCCR2B |= (1 << CS22);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
}

void Callback(void *arg)
{
  unsigned int pin = (unsigned int)arg;
  static unsigned char level = HIGH;

  digitalWrite(pin, level);
  if (level == HIGH) {
    level = LOW;
  } else {
    level = HIGH;
  }
}

void setup() {
  nOS_Init();

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  nOS_Start(Timer2Init);

  nOS_TimerCreate(&timer, Callback, (void *)13, 500, NOS_TIMER_FREE_RUNNING);
  nOS_TimerStart(&timer);
}

void loop() {
}
