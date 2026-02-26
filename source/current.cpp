#include "ch.h"
#include "hal.h"
#include "current.h"

#include <stdutil++.hpp>
#include "ttyConsole.h"



#define ADC_GRP1_NUM_CHANNELS 1
#define ADC_GRP1_BUF_DEPTH   1

static adcsample_t adc_buffer[ADC_GRP1_BUF_DEPTH * ADC_GRP1_NUM_CHANNELS];

static const ADCConfig portab_adccfg1 = {
  .difsel = 0U
};

static void adc_error_cb(ADCDriver *adcp, adcerror_t err) {
  (void)adcp;
  (void)err;
  chSysHalt("C'est l'adc qui marche pas ");
}

static const ADCConversionGroup portab_adcgrpcfg1 = {
  .circular     = false,
  .num_channels = ADC_GRP1_NUM_CHANNELS,
  .end_cb       = NULL,
  .error_cb     = adc_error_cb,
  .cfgr         = 0U,
  .cfgr2        = 0U,
  .tr1          = ADC_TR_DISABLED,
  .tr2          = ADC_TR_DISABLED,
  .tr3          = ADC_TR_DISABLED,
  .awd2cr       = 0U,
  .awd3cr       = 0U,
  .smpr         = {
    ADC_SMPR1_SMP_AN6(ADC_SMPR_SMP_247P5),
    0U
  },
  .sqr          = {
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN6),
    0U,
    0U,
    0U
  }
};


static uint16_t currentVoltage = 0;
static mutex_t currentMutex;

static THD_WORKING_AREA(waCurrentAdc, 1024);
static THD_FUNCTION(CurrentAdcThread, arg) {
  (void)arg;
  chRegSetThreadName("Current_ADC");

  adcStart(&ADCD1, &portab_adccfg1);

  while (true) {
    float alpha = 0.85; 
    adcConvert(&ADCD1,
               &portab_adcgrpcfg1,
               adc_buffer,
               ADC_GRP1_BUF_DEPTH);

    chMtxLock(&currentMutex);
    currentVoltage = alpha * currentVoltage + (1- alpha) * adc_buffer[0];
    chMtxUnlock(&currentMutex);
    DebugTrace(">current:%u", currentVoltage); 
    chThdSleepMilliseconds(100);
  }
}

void currentInit(void) {
  chMtxObjectInit(&currentMutex);
  chThdCreateStatic(waCurrentAdc,
                    sizeof(waCurrentAdc),
                    NORMALPRIO,
                    CurrentAdcThread,
                    NULL);
}

uint16_t getCurrent(void) {
  uint16_t val;

  chMtxLock(&currentMutex);
  val = currentVoltage;
  chMtxUnlock(&currentMutex);

  return val;
}
