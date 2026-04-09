#include <ch.h>
#include <hal.h>
#include <stdutil++.hpp>
#include "ttyConsole.h"
#include "current.h"
#include "uart.h"
#include "pompe_valve.h"
#include "settings.h"

uint16_t wait_led = 1000; 

static const WDGConfig wdgcfg = {
  .pr           = STM32_IWDG_PR_64,
  .rlr          = STM32_IWDG_RL(1000),
#if STM32_IWDG_IS_WINDOWED
  .winr         = STM32_IWDG_WIN_DISABLED,
#endif
};


static THD_WORKING_AREA(waBlinker, 1024);
static THD_FUNCTION(Blinker, arg) {
  (void)arg; 
  chRegSetThreadName("Blinker");
  if (RCC->CSR & RCC_CSR_IWDGRSTF) {
    wait_led = 100; 
  } 
  while (true) {   
    palToggleLine(LINE_LED);
    chThdSleepMilliseconds(wait_led);
    wdgReset(&WDGD1);
  }  
}

int main(void) {

  halInit();
  chSysInit();

  wdgStart(&WDGD1, &wdgcfg);

  // consoleInit();
  // consoleLaunch();

  currentInit();     
  uartCmdInit();  
  pumpInit();
  pump_off();
  valve_fermer();

  chThdCreateStatic(waBlinker,sizeof(waBlinker), LOWPRIO, Blinker, NULL);

  
  chThdSleep(TIME_INFINITE);
}
