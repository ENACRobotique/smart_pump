#include <ch.h>
#include <hal.h>
#include "pompe_valve.h"




// pompe + valve gestion

void pump_on(void) {
    palSetLine(LINE_PUMP);
   // palSetLine(LINE_LED); 
    //chThdSleepMilliseconds(100);
    //palToggleLine(LINE_LED); 
    //chThdSleepMilliseconds(100);
  }
  
  void pump_off(void) {
    palClearLine(LINE_PUMP);
    palSetLine(LINE_LED); 
    chThdSleepMilliseconds(100);
    palToggleLine(LINE_LED); 
    chThdSleepMilliseconds(100);
  }
  
  void valve_ouvert(void) {
    palSetLine(LINE_VALVE);
    palSetLine(LINE_LED); 
    chThdSleepMilliseconds(1000);
  }
  
  void valve_fermer(void) {
    palClearLine(LINE_VALVE);
    palSetLine(LINE_LED); 
    chThdSleepMilliseconds(1000);
  }
  void valve_utilisation(void){
    palSetLine(LINE_VALVE);
    palSetLine(LINE_LED); 
    chThdSleepMilliseconds(50); 
    palToggleLine(LINE_LED);
    chThdSleepMilliseconds(50); 
    palToggleLine(LINE_LED);
    chThdSleepMilliseconds(50); 
    palToggleLine(LINE_LED);
    chThdSleepMilliseconds(50); 
    palToggleLine(LINE_LED);
    chThdSleepMilliseconds(300); 
    palClearLine(LINE_VALVE); 
  }