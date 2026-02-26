#include "ch.h"
#include "hal.h"
#include "uart.h"
#include "pompe_valve.h"
#include "current.h"
#include <string.h>
#include <stdutil++.hpp>
#include "ttyConsole.h"
#include "settings.h"

rom_settings_t settings;
extern uint16_t wait_led;

static SerialConfig uartCfg = {
    .speed = 115200,
    .cr1 = 0,
    .cr2 = USART_CR2_STOP1_BITS,
    .cr3 = USART_CR3_HDSEL};

typedef struct
{
    uint8_t value;
    uint32_t baudrate;
} baudrate_map_t;

static baudrate_map_t baudrate_map[9] = {
    {1, 1000000},
    {3, 500000},
    {4, 400000},
    {7, 250000},
    {9, 200000},
    {16, 115200},
    {34, 57600},
    {103, 19200},
    {207, 9600},
};

static uint32_t get_baudrate(uint8_t value)
{
    for (int i = 0; i < 9; i++)
    {
        if (value == baudrate_map[i].value)
        {
            return baudrate_map[i].baudrate;
        }
    }
    return 0;
}

void wait_return_delay(void)
{
    if (settings.return_delay > 0)
{
        chThdSleepMicroseconds(settings.return_delay * 2);
    }
}

static void send_status_packet(uint8_t error, uint8_t *params, uint8_t param_lenght, bool repond)
{

    if(repond == false){
        return; 
    }
    uint8_t buffer[32];

    buffer[0] = 0xFF;
    buffer[1] = 0xFF;
    buffer[2] = settings.id;
    buffer[3] = param_lenght + 2;
    buffer[4] = error;

    uint8_t checksum_calcul = buffer[2] + buffer[3] + buffer[4];
    for (int i = 0; i < param_lenght; i++)
    {
        buffer[5 + i] = params[i];
        checksum_calcul += params[i];
    }
    buffer[5 + param_lenght] = ~checksum_calcul;

    wait_return_delay();

    sdWrite(&SD2, buffer, param_lenght + 6);
}

static void action(uint8_t instruction, uint8_t *params, uint8_t param_len, bool repond)
{
    uint8_t tx_params[4];
    uint8_t error = 0;
    


    switch (instruction)
    {
    case PING: 
        send_status_packet(0, NULL, 0, repond);
        break;

    case WRITE:
        if (param_len >= 2)
        {
            uint8_t reg = params[0]; // Adresse du registre
            uint8_t val = params[1]; // Valeur

            // --- MODIFICATION : Gestion du Return Delay Time ---
            if (reg == REG_RETURN_TIME)
            {
                settings.return_delay = val;
                mfs_error_t status =  store_settings(1, &settings);
                DebugTrace ("j'ai changer le return time %d", status); 
            }
            // --------------------------------------------------
            else if (reg == REG_POMPE)
            {
                if (val != 0)
                {
                    pump_on();
                    DebugTrace("Pump ON\r\n");
                }
                else
                {
                    pump_off();
                    DebugTrace("Pump OFF\r\n");
                }
            }
            else if (reg == REG_VALVE)
            {
                if (val != 0)
                {
                    valve_ouvert();
                    DebugTrace("Valve OPEN\r\n");
                }
                else
                {
                    valve_fermer();
                    DebugTrace("Valve CLOSE\r\n");
                }
            }
            else if (reg == REG_VALVE_USE)
            {
                send_status_packet(error, NULL, 0 , repond) ;
                valve_utilisation();
                DebugTrace("Valve utiliser\r\n");
                return;
            }
            else if (reg == REG_ID)
            {
                if (params[1] == 0 || params[1] == 255 || params[1] == 254)
                {
                    error = error | 1 << 3;
                }
                else
                {
                    settings.id= params[1]; 
                    mfs_error_t status =  store_settings(1, &settings);
                    DebugTrace("status = %d, param[1] = %d , setting.id = %d \r\n", status, params[1], settings.id);
        
                }
            }
            else if (reg == REG_BAUDRATE)
            {
                uint32_t baudrate_speed = get_baudrate(params[1]);
                if (baudrate_speed != 0)
                {
                    settings.baudrate = params[1];
                    mfs_error_t status = store_settings(1, &settings);
                    DebugTrace("J'ai bien changer le bauderate je redémarre %d \r\n", status);
                    sdStop(&SD2);
                    uartCfg.speed = baudrate_speed;
                    sdStart(&SD2, &uartCfg);
                }
                else
                {
                    error = error | 1 << 3;
                }
            }
            else if (reg == REG_RETURN_TIME)
            {
                    settings.return_delay = params[1]; 
                    store_settings(1, &settings);
            }
            // else if(reg == REG_CURRENT_TRESHOLD_LSB){
            //     if(params[1]!= 0){
            //         settings.current_threshold
            //     }
            // }
        }
        send_status_packet(error, NULL, 0 , repond) ;

        break;

    case READ:

        if (param_len == 2)
        {

            if (params[0] == REG_CURRENT_LSB || params[1] == 2)
            {
                uint16_t current = getCurrent();
                DebugTrace("Courant de l'ADC est de %f \r\n", current);

                tx_params[0] = current & 0xFF;
                tx_params[1] = (current>>8) & 0xFF  ;

                send_status_packet(0, tx_params, 2 , repond);
            }
        }
        break;
    }
}

static THD_WORKING_AREA(waUartCmd, 1024);
static THD_FUNCTION(UartCmdThread, arg)
{
    (void)arg;
    chRegSetThreadName("UART_CMD");

    uint8_t rx_bit;
    uint8_t state = 0;
    uint8_t id = 0;
    uint8_t lenght = 0;
    uint8_t instruction = 0;
    uint8_t checksum = 0;
    uint8_t params[16];
    uint8_t param_idx = 0;

    while (true)
    {
        rx_bit = sdGet(&SD2);

        switch (state)
        {
        case 0:
            if (rx_bit == 0xFF)
                state = 1;
            break;
        case 1:
            if (rx_bit == 0xFF)
                state = 2;
            else
                state = 0;
            break;
        case 2:
            if (rx_bit == 0xFF)
            {
                state = 2;
            }
            else
            {
                id = rx_bit;
                state = 3;
            }
            break;
        case 3:
            lenght = rx_bit;
            param_idx = 0;
            state = 4;
            break;
        case 4:
            instruction = rx_bit;
            if (lenght > 2)
            {
                state = 5;
            }
            else
                state = 6;
            break;
        case 5:
            params[param_idx++] = rx_bit;
            if (param_idx >= (lenght - 2))
            {
                state = 6;
            }
            break;
        case 6:
            checksum = rx_bit;

            if (id == settings.id || id == 0xFE)
            {
                uint8_t calcul_checksum = id + lenght + instruction;
                for (int i = 0; i < (lenght - 2); i++)
                {
                    calcul_checksum += params[i];
                }
                calcul_checksum = ~calcul_checksum;

                if (calcul_checksum == checksum)
                {
                    bool repond = (id != 0xFE) || (instruction == PING);  
                    action(instruction, params, lenght - 2, repond);
                }
            }

            state = 0;
            break;
        }
    }
}

void uartCmdInit(void)
{
    
    settingsInit();
    mfs_error_t status = read_settings(1, &settings);
    if (status != MFS_NO_ERROR)
    {
        wait_led = 100;
        settings = {
            .id = 0x01,
            .baudrate = 16,
            .return_delay = 254,
            .pump_duty = 100,
            .valve_duty = 100,
            .current_threshold = 300,
            .valve_release_time = 500,
        };
    }
    DebugTrace("id = %d, \r\n baudrate = %d,\r\n return_delay = %d,\r\n pump_duty =%d, \r\n valve_duty = %d,\r\n current_threshold = %d,\r\n valve_release_time = %d \r\n",
        settings.id, settings.baudrate, settings.return_delay, settings.pump_duty, settings.valve_duty, settings.current_threshold, settings.valve_release_time); 
    uint32_t baudrate_speed = get_baudrate(settings.baudrate);
    if (baudrate_speed == 0)
    {
        baudrate_speed = 115200;
    }
    uartCfg.speed = baudrate_speed;
    sdStart(&SD2, &uartCfg);
    chThdCreateStatic(
        waUartCmd,
        sizeof(waUartCmd),
        NORMALPRIO,
        UartCmdThread,
        NULL);
}