#include <ch.h>
#include <hal.h>
#include "pompe_valve.h"




// pompe + valve gestion

void pump_on(void) {
    palSetLine(LINE_PUMP);
  }
  
  void pump_off(void) {
    palClearLine(LINE_PUMP);
  }
  
  void valve_ouvert(void) {
    palSetLine(LINE_VALVE);
  }
  
  void valve_fermer(void) {
    palClearLine(LINE_VALVE);
  }


  void valve_utilisation(void){
    palSetLine(LINE_VALVE);
    chThdSleepMilliseconds(500); 
    palClearLine(LINE_VALVE); 
  }