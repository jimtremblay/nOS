#include "nOS.h"

nOS_Timer timer;
nOS_Thread threadLow;
nOS_Thread threadHigh;
nOS_Sem semLow;
nOS_Sem semHigh;
nOS_Stack stackLow[128];
nOS_Stack stackHigh[128];

struct Config {
  unsigned char pin;
  unsigned char level;
  nOS_Sem *sem;
} configLow, configHigh;

NOS_ISR(TIMER2_COMPA_vect)
{
  nOS_Tick();
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

void Thread (void *arg)
{
  struct Config *config = (struct Config *)arg;

  while (1) {
    nOS_SemTake(config->sem, NOS_WAIT_INFINITE);
    digitalWrite(config->pin, config->level);
  }
}

void setup() {
  nOS_Init();

  nOS_SemCreate(&semLow, 0, 1);
  configLow.pin = 13;
  configLow.level = LOW;
  configLow.sem = &semLow;

  nOS_SemCreate(&semHigh, 0, 1);
  configHigh.pin = 13;
  configHigh.level = HIGH;
  configHigh.sem = &semHigh;

  nOS_ThreadCreate(&threadLow,  Thread, (void*)&configLow,  stackLow,  128, 7, NOS_THREAD_READY);
  nOS_ThreadCreate(&threadHigh, Thread, (void*)&configHigh, stackHigh, 128, 7, NOS_THREAD_READY);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  nOS_Start(Timer2Init);
}

void loop() {
  delay(500);
  nOS_SemGive(&semHigh);
  delay(500);
  nOS_SemGive(&semLow);
}
