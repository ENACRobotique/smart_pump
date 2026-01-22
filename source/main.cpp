
#include <ch.h>
#include <hal.h>
#include "ttyConsole.h"
#include "stdutil++.hpp"

int main(void) {
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();
  initHeap();


  consoleInit(); // initialisation de la liaison série du shell
  consoleLaunch();  // lancement du thread qui gère le shell sur la liaison série

  while(1) {
    palToggleLine(LINE_LED);
    chThdSleepMilliseconds(500);
  }

  // main thread does nothing
  chThdSleep(TIME_INFINITE);
}


