#include "ch.h"
#include "hal.h"
#include "uart.h"
#include "pompe_valve.h"
#include "current.h"
#include <string.h>
#include <stdutil++.hpp>
#include "ttyConsole.h"
#include "settings.h"

#define TIMEOUT chTimeMS2I(1)


constexpr uint16_t MODEL_NUMBER = 20;

rom_settings_t settings;
extern uint16_t wait_led;

static uint8_t buffer[32];
static uint8_t param_lenght = 0;

static SerialConfig uartCfg = {
    .speed = 500000,
    .cr1 = 0,
    .cr2 = USART_CR2_STOP1_BITS,
    .cr3 = USART_CR3_HDSEL
};

typedef struct {
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
    for (int i = 0; i < 9; i++) {
        if (value == baudrate_map[i].value) {
            return baudrate_map[i].baudrate;
        }
    }
    return 0;
}


static void uart_timer_callback(GPTDriver *gptp) {
    (void)gptp;

    chSysLockFromISR();
    sdWriteI(&SD2, buffer, param_lenght + 6);
    chSysUnlockFromISR();
}

static const GPTConfig gpt16cgf = {
    500000U,
    uart_timer_callback,
    0,
    0};

static void send_status_packet(uint8_t error, uint8_t *params, bool repond) {

    if (repond == false)
    {
        return;
    }

    buffer[0] = 0xFF;
    buffer[1] = 0xFF;
    buffer[2] = settings.id;
    buffer[3] = param_lenght + 2;
    buffer[4] = error;

    uint8_t checksum_calcul = buffer[2] + buffer[3] + buffer[4];
    for (int i = 0; i < param_lenght; i++) {
        buffer[5 + i] = params[i];
        checksum_calcul += params[i];
    }
    buffer[5 + param_lenght] = ~checksum_calcul;

    gptStartOneShot(&GPTD16, settings.return_delay);
}

static void action(uint8_t instruction, uint8_t *params, uint8_t param_len, bool repond) {
    uint8_t tx_params[4];
    uint8_t error = 0;

    switch (instruction)
    {
    case PING:
        param_lenght = 0;
        send_status_packet(0, NULL, repond);
        break;

    case WRITE:
        if (param_len >= 2)
        {
            uint8_t reg = params[0]; // Adresse du registre
            if (reg == REG_ID) {
                if (params[1] == 0 || params[1] == 255 || params[1] == 254)            
{
                    error = error | 1 << 3;
                }
                else
                {
                    settings.id = params[1];
                    store_settings(1, &settings);
                    DebugTrace("param[1] = %d , setting.id = %d \r\n", params[1], settings.id);
                }
            }
            else if (reg == REG_RETURN_TIME) {
                settings.return_delay = params[1];
                store_settings(1, &settings);
            }
            else if (reg == REG_BAUDRATE) {
                uint32_t baudrate_speed = get_baudrate(params[1]);
                if (baudrate_speed != 0)
                {
                    settings.baudrate = params[1];
                    store_settings(1, &settings);
                    DebugTrace("J'ai bien changer le baudrate je redémarre\r\n");
                    sdStop(&SD2);
                    uartCfg.speed = baudrate_speed;
                    sdStart(&SD2, &uartCfg);
                }
                else
                {
                    error = error | 1 << 3;
                }
            }
            else if (reg == REG_PUMP_DUTY) {
                settings.pump_duty = params[1];
                store_settings(1, &settings);
            }
            else if (reg == REG_VALVE_DUTY) {
                settings.valve_duty = params[1];
                store_settings(1, &settings);
            }
            else if (reg == REG_VALVE_RELEASE_TIME) {
                settings.valve_release_time = params[1];
                store_settings(1, &settings);
            }
            else if (reg == REG_POMPE) {
                if (params[1] != 0)
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
            else if (reg == REG_VALVE) {
                if (params[1] != 0)
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
            else if (reg == REG_VALVE_USE) {
                param_lenght = 0;
                send_status_packet(error, NULL, repond);
                valve_utilisation();
                DebugTrace("Valve utiliser\r\n");
                return;
            }
        }
        param_lenght = 0;
        send_status_packet(error, NULL, repond);

        break;

    case READ:

        if (param_len == 2)
        {
            uint8_t reg = params[0];
            uint8_t nb_params = params[1];
            if (reg == REG_MODEL_NUMBER_LSB && nb_params == 2)
            {
                uint16_t *p_model_number = (uint16_t *)tx_params;
                *p_model_number = MODEL_NUMBER;
                param_lenght = 2;
                send_status_packet(0, tx_params, repond);
            }
            else if (reg == REG_ID && nb_params == 1) {
                tx_params[0] = settings.id;
                param_lenght = 1;
                send_status_packet(0, tx_params, repond);
            }
            else if (reg == REG_RETURN_TIME && nb_params == 1) {
                tx_params[0] = settings.return_delay;
                param_lenght = 1;
                send_status_packet(0, tx_params, repond);
            }
            else if (reg == REG_BAUDRATE && nb_params == 1) {
                tx_params[0] = settings.baudrate;
                param_lenght = 1;
                send_status_packet(0, tx_params, repond);
            }
            else if (reg == REG_PUMP_DUTY && nb_params == 1) {
                tx_params[0] = settings.pump_duty;
                param_lenght = 1;
                send_status_packet(0, tx_params, repond);
            }
            else if (reg == REG_VALVE_DUTY && nb_params == 1) {
                tx_params[0] = settings.valve_duty;
                param_lenght = 1;
                send_status_packet(0, tx_params, repond);
            }
            else if (reg == REG_VALVE_RELEASE_TIME && nb_params == 1) {
                tx_params[0] = settings.valve_release_time;
                param_lenght = 1;
                send_status_packet(0, tx_params, repond);
            }
            else if (reg == REG_POMPE && nb_params == 1) {
                tx_params[0] = getPumpState(); 
                param_lenght = 1;
                send_status_packet(0, tx_params, repond);
            }
            else if (reg == REG_VALVE && nb_params == 1) {
                tx_params[0] = getValveState(); 
                param_lenght = 1;
                send_status_packet(0, tx_params, repond);
            }
            else if (reg == REG_CURRENT_LSB && nb_params == 2) {
                uint16_t *p_current = (uint16_t *)&tx_params[0];
                *p_current = getCurrent();
                DebugTrace("Courant de l'ADC est de %d \r\n", *p_current);
                param_lenght = 2;
                send_status_packet(0, tx_params, repond);
            }
            else {
                param_lenght = 0;
                send_status_packet(error, NULL, repond);
            }
        } else {

        }
        break;
    }
}

static THD_WORKING_AREA(waUartCmd, 1024);
static THD_FUNCTION(UartCmdThread, arg)
{
    (void)arg;
    chRegSetThreadName("UART_CMD");

    uint8_t state = 0;
    uint8_t id = 0;
    uint8_t lenght = 0;
    uint8_t instruction = 0;
    uint8_t checksum = 0;
    uint8_t params[16];
    uint8_t param_idx = 0;

    while (true)
    {
          
        msg_t rx_bit = sdGetTimeout(&SD2,TIMEOUT);
        if (rx_bit < 0){
            state = 0; 
            continue; 
        }

        switch (state)
        {
        case 0:
            if (rx_bit == 0xFF)
                state = 1;
            break;
        case 1:
            if (rx_bit == 0xFF)
            {
                state = 2;
            }
            else
            {
                state = 0;
            }
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
    gptStart(&GPTD16, &gpt16cgf);
    settingsInit();
    mfs_error_t status = read_settings(1, &settings);
    if (status != MFS_NO_ERROR)
    {
        wait_led = 100;
        settings = {
            .id = 0x01,
            .baudrate = 3,
            .return_delay = 254,
            .pump_duty = 100,
            .valve_duty = 100,
            .current_threshold = 300,
            .valve_release_time = 5,
        };
    }
    DebugTrace("id = %d, \r\n baudrate = %d,\r\n return_delay = %d,\r\n pump_duty =%d, \r\n valve_duty = %d,\r\n current_threshold = %d,\r\n valve_release_time = %d \r\n",
               settings.id, settings.baudrate, settings.return_delay, settings.pump_duty, settings.valve_duty, settings.current_threshold, settings.valve_release_time);
    uint32_t baudrate_speed = get_baudrate(settings.baudrate);
    if (baudrate_speed == 0)
    {
        baudrate_speed = 500000;
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