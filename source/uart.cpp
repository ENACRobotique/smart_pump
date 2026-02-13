#include "ch.h"
#include "hal.h"
#include "uart.h"
#include "pompe_valve.h"
#include "current.h"
#include <string.h>
#include <stdutil++.hpp>
#include "ttyConsole.h"

static const SerialConfig uartCfg = {
    .speed = 115200,
    .cr1   = 0,
    .cr2   = USART_CR2_STOP1_BITS,
    .cr3   = USART_CR3_HDSEL
};

static THD_WORKING_AREA(waUartCmd, 1024);
static THD_FUNCTION(UartCmdThread, arg) {
    (void)arg;
    chRegSetThreadName("UART_CMD");

    while (true) {
        uint8_t c = sdGet(&SD2) ;

      
            if (c == 'o') {
                pump_on();
                DebugTrace("Pump ON\r\n");

            } else if (c == 'p') {
                pump_off();
                DebugTrace("Pump OFF\r\n");

            } else if (c ==  'v') {
                valve_ouvert();
                DebugTrace("Valve OPEN\r\n");

            } else if (c == 'b') {
                valve_fermer();
                DebugTrace("Valve CLOSE\r\n");
            }
            else if (c == 't'){
                valve_utilisation();
                DebugTrace("Valve utiliser\r\n");
            }
            else if (c == 'm'){
                float current = getCurrent(); 
                DebugTrace ("Courant de l'ADC est de %f", current); 
            }
            
            }
    }



void uartCmdInit(void) {
    sdStart(&SD2, &uartCfg);

    chThdCreateStatic(
        waUartCmd,
        sizeof(waUartCmd),
        NORMALPRIO,
        UartCmdThread,
        NULL
    );
}
