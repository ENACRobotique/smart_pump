#include <ch.h>
#include <hal.h>
#include <stdutil++.hpp>
#include "ttyConsole.h"
#include "current.h"
#include "uart.h"
#include "pompe_valve.h"


static THD_WORKING_AREA(waBlinker, 1024);
static THD_FUNCTION(Blinker, arg) {
  (void)arg; 
  chRegSetThreadName("Blinker");

  while (true) {   
    palToggleLine(LINE_LED);
    chThdSleepMilliseconds(500);
  }  
}

int main(void) {

  halInit();
  chSysInit();

  
  consoleInit();
  consoleLaunch();
  pump_off();
  valve_fermer();

  
  currentInit();     
  uartCmdInit();    

  chThdCreateStatic(waBlinker,sizeof(waBlinker), LOWPRIO, Blinker, NULL);

  
  chThdSleep(TIME_INFINITE);
}
