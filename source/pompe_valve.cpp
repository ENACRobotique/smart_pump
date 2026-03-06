#include <ch.h>
#include <hal.h>
#include "pompe_valve.h"
#include "settings.h"

const PWMConfig pwm_conf = {
  .frequency = 10000,
  .period = 100,
  .callback = NULL,
  .channels = {
    {.mode = PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, .callback = NULL},
    {.mode = PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, .callback = NULL},
    {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
    {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
  },
  .cr2 = 0,
  .bdtr = 0,
  .dier = 0
};

extern rom_settings_t settings;


// pompe + valve gestion

void pump_on(void) {
  pwmEnableChannel(&PWMD1, 1, settings.pump_duty);
  //palSetLine(LINE_PUMP);
}

void pump_off(void) {
  pwmEnableChannel(&PWMD1, 1, 0);
  //palClearLine(LINE_PUMP);
}

void valve_ouvert(void) {
  pwmEnableChannel(&PWMD1, 0, settings.valve_duty);
  palToggleLine(LINE_LED);
  // palSetLine(LINE_VALVE);
}

void valve_fermer(void) {
  pwmEnableChannel(&PWMD1, 0, 0);
  // palClearLine(LINE_VALVE);
}


void valve_utilisation(void){
  valve_ouvert();
  chThdSleepMilliseconds(settings.valve_release_time); 
  valve_fermer();
  // palSetLine(LINE_VALVE);
  // chThdSleepMilliseconds(500); 
  // palClearLine(LINE_VALVE); 
}

void pumpInit() {
  pwmStart(&PWMD1, &pwm_conf);
}