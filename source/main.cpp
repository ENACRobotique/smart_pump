#include <ch.h>
#include <hal.h>
#include "adcHelper.h"

#define LINE_PUMP   PAL_LINE(GPIOB, 0)
#define LINE_VALVE  PAL_LINE(GPIOA, 7)
#define LINE_LED    PAL_LINE(GPIOB, 3)

// rajouter halconf et muconf adc

#define ADC_ARRET  2500  // il faut définir ça
#define ADC_CHANNEL  1
#define ADC_BUFFER   8

static adcsample_t adc_buffer[ADC_BUFFER];

static const ADCConversionGroup adcCfg = adcgetConfig (
2, 
LINE_CO1_POT1, 
LINE_CO2_POT2,
ADC_END 
);


static bool pump_enabled = false;

// pompe + valve gestion

static inline void pump_on(void) {
  palSetLine(LINE_PUMP);
}

static inline void pump_off(void) {
  palClearLine(LINE_PUMP);
}

static inline void valve_ouvert(void) {
  palSetLine(LINE_VALVE);
}

static inline void valve_fermer(void) {
  palClearLine(LINE_VALVE);
}

// init adc

static uint16_t lecture_courant(void) {
  adcConvert(&ADC1, &adc_convert, adc_buffer, ADC_BUFFER);

  uint32_t sum = 0;
  for (uint8_t i = 0; i < ADC_BUFFER; i++) {
    sum += adc_buffer[i];
  }
  return sum / ADC_BUFFER;
}



// thread pour l'adc
static THD_WORKING_AREA(waAdcWatch, 256);
static THD_FUNCTION(AdcWatchThread, arg) {
  (void)arg;
  chRegSetThreadName("ADC_Watch");

  while (true) {
    if (pump_enabled) {
      uint16_t current = read_current_adc();

      if (current > CURRENT_THRESHOLD) {
        pump_off();
        pump_enabled = false;
      }
    }
    chThdSleepMilliseconds(100);
  }
}

// UART

static SerialConfig serial_cfg = {
  115200,
  0,
  USART_CR2_STOP1_BITS,
  0
};

/* ---- Commande pompe ---- */
static void cmd_pump(BaseSequentialStream *chp, int argc, char *argv[]) {

  if (argc != 1) {
    chprintf(chp, "Usage: pump on\r\n");
    return;
  }

  if (!strcmp(argv[0], "on")) {
    pump_on();
    pump_enabled = true;
    chprintf(chp, "Pump ON\r\n");
  }
}

/* ---- Commande valve ---- */
static void cmd_valve(BaseSequentialStream *chp, int argc, char *argv[]) {

  if (argc != 1) {
    chprintf(chp, "Usage: valve open | close\r\n");
    return;
  }

  if (!strcmp(argv[0], "open")) {
    valve_ouverte();
    chprintf(chp, "Valve ouverte\r\n");

  } else if (!strcmp(argv[0], "close")) {
    valve_fermer();
    chprintf(chp, "Valve Fermer\r\n");
  }
}

/* ---- Shell ---- */
static const ShellCommand commands[] = {
  {"pump",  cmd_pump},
  {"valve", cmd_valve},
  {NULL, NULL}
};

static const ShellConfig shell_cfg = {
  (BaseSequentialStream *)&SD2,
  commands
};



int main(void) {

  halInit();
  chSysInit();

  sdStart(&SD2, &serial_cfg);
  adcStart(&ADCD1, NULL);

  pump_off();
  valve_fermer();


  chThdCreateStatic(waAdcWatch, sizeof(waAdcWatch),
                    NORMALPRIO, AdcWatchThread, NULL);

  shellInit();
  shellCreate(&shell_cfg, THD_WORKING_AREA_SIZE(1024),
              NORMALPRIO);

  while (true) {
    palToggleLine(LINE_LED);
    chThdSleepMilliseconds(500);
  }
}
