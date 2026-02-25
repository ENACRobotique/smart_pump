#include <ch.h>
#include <hal.h>
#include <stdutil++.hpp>
#include "ttyConsole.h"
#include "current.h"
#include "uart.h"
#include "pompe_valve.h"
#include "settings.h"

uint16_t wait_led = 1000; 

static THD_WORKING_AREA(waBlinker, 1024);
static THD_FUNCTION(Blinker, arg) {
  (void)arg; 
  chRegSetThreadName("Blinker");

  while (true) {   
    palToggleLine(LINE_LED);
    chThdSleepMilliseconds(wait_led);
  }  
}

int main(void) {

  halInit();
  chSysInit();

  // consoleInit();
  // consoleLaunch();
  pump_off();
  valve_fermer();

  
  currentInit();     
  uartCmdInit();    

  chThdCreateStatic(waBlinker,sizeof(waBlinker), LOWPRIO, Blinker, NULL);

  
  chThdSleep(TIME_INFINITE);
}
