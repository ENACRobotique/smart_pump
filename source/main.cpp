#include <ch.h>
#include <hal.h>
#include <stdutil++.hpp>
#include "ttyConsole.h"


#define ADC_GRP1_NUM_CHANNELS 1
#define ADC_GRP1_BUF_DEPTH   8

static adcsample_t adc_buffer[ADC_GRP1_BUF_DEPTH*ADC_GRP1_NUM_CHANNELS];

const ADCConfig portab_adccfg1 = {
  .difsel       = 0U
}; 

const ADCConversionGroup portab_adcgrpcfg1 = {
  .circular     = false,
  .num_channels = ADC_GRP1_NUM_CHANNELS,
  .end_cb       = NULL,
  .error_cb     = NULL, //adcerrorcallback,
  .cfgr         = 0U,
  .cfgr2        = 0U,
  .tr1          = ADC_TR_DISABLED,
  .tr2          = ADC_TR_DISABLED,
  .tr3          = ADC_TR_DISABLED,
  .awd2cr       = 0U,
  .awd3cr       = 0U,
  .smpr         = {
    ADC_SMPR1_SMP_AN0(ADC_SMPR_SMP_247P5) |
    ADC_SMPR1_SMP_AN5(ADC_SMPR_SMP_247P5),
    0U
  },
  .sqr          = {
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN0) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN5),
    0U,
    0U,
    0U
  }
};




double ValeurVolt; 
// thread pour l'adc
static THD_WORKING_AREA(waAdcWatch, 1024);
static THD_FUNCTION(AdcWatchThread, arg) {
  (void)arg; 
  chRegSetThreadName("ADC_Watch");

  adcStart(&ADCD1,NULL);
    
  while(true){
    adcConvert(&ADCD1, &portab_adcgrpcfg1, adc_buffer, ADC_GRP1_BUF_DEPTH ); 

    const adcsample_t Sample = adc_buffer[0]; 
    ValeurVolt = (Sample *3.3)/4096; 
    DebugTrace("%f", ValeurVolt);

    chThdSleepMilliseconds(100);
  }
  
}



// UART

static const SerialConfig terminalConfig = {
  .speed = 115200,
  .cr1 = 0,
  .cr2 = USART_CR2_STOP1_BITS, //    | USART_CR2_LINEN, 
  .cr3 = 0 // USART_CR3_HDSEL  // pour le half duplex T
};



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

// /* ---- Commande pompe ---- */
// static void cmd_pump(BaseSequentialStream *chp, int argc, char *argv[]) {

//   if (argc != 1) {
//     DebugTrace("Usage: pump on\r\n");
//     return;
//   }

//   if (!strcmp(argv[0], "on")) {
//     pump_on();

//     DebugTrace("Pump ON\r\n");
//   }
// }

// /* ---- Commande valve ---- */
// static void cmd_valve(BaseSequentialStream *chp, int argc, char *argv[]) {

//   if (argc != 1) {
//     chprintf(chp, "Usage: valve open | close\r\n");
//     return;
//   }

//   if (!strcmp(argv[0], "open")) {
//     valve_ouverte();
//     Debugtrace( "Valve ouverte\r\n");

//   } else if (!strcmp(argv[0], "close")) {
//     valve_fermer();
//     DebugTrace( "Valve Fermer\r\n");
//   }
// }
static THD_WORKING_AREA(waBlinker, 1024);
static THD_FUNCTION(Blinker, arg) {
  (void)arg; 
  chRegSetThreadName("Blinker");

  while (true) {   
    palToggleLine(LINE_LED);
    DebugTrace( "Test");
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

  chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO, Blinker, NULL);
  chThdCreateStatic(waAdcWatch, sizeof(waAdcWatch), NORMALPRIO, AdcWatchThread, NULL);

 chThdSleep(TIME_INFINITE);

}
