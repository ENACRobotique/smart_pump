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

static const GPTConfig gpt15cgf = {
  10000u,
  valve_timer_callback,
  0,
  0
};

extern rom_settings_t settings;
static volatile uint8_t etat_valve; 
static volatile uint8_t etat_pompe;

bool getPumpState() {
  return etat_pompe;
}

bool getValveState() {
  return etat_valve;
}

// pompe + valve gestion

void pump_on(void) {
  pwmEnableChannel(&PWMD1, 1, settings.pump_duty);
  etat_pompe = 1; 
}

void pump_off(void) {
  pwmEnableChannel(&PWMD1, 1, 0);
  etat_pompe = 0; 
}

void valve_ouvert(void) {
  pwmEnableChannel(&PWMD1, 0, settings.valve_duty);
  palToggleLine(LINE_LED);
  etat_valve = 1; 
}

void valve_fermer(void) {
  pwmEnableChannel(&PWMD1, 0, 0);
  etat_valve =0; 
}

static void valve_timer_callback(GPTDriver*) {
  chSysLockFromISR();
  pwmEnableChannelI(&PWMD1, 0, 0);
  etat_valve = 0;
  chSysUnlockFromISR();
}


void valve_utilisation(void) {
  gptStopTimer(&GPTD15);
  valve_ouvert();
  gptStartOneShot(&GPTD15, settings.valve_release_time*1000);
}

void pumpInit() {
  pwmStart(&PWMD1, &pwm_conf);
  gptStart(&GPTD15, &gpt15cgf);
}