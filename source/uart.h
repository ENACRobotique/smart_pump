#ifndef UART_CMD_H
#define UART_CMD_H

#define HEADER  0xFF
#define PING    0x01
#define READ    0x02
#define WRITE   0x03

#define REG_HW_VERSION 0X00
#define REG_SW_VERSION 0X01
#define REG_ID 0X02
#define REG_CURRENT_TRESHOLD_LSB 0x03  // two bytes long
#define REG_CURRENT_TRESHOLD_MSB 0x04
#define REG_RETURN_TIME 0x05
#define REG_BAUDRATE    0x06
#define REG_PUMP_DUTY    0X07
#define REG_VALVE_DUTY  0X08
#define REG_VALVE_RELEASE_TIME 0X09

#define REG_POMPE 0x20 // registre de reception des commande 
#define REG_VALVE 0x21
#define REG_VALVE_USE 0x22
#define REG_CURRENT_LSB 0x23
#define REG_CUSSENT_MSB 0x24 
#define REG_GRAB_OK 0x25




void uartCmdInit(void);
void action(uint8_t instruction, uint8_t *params, uint8_t param_len); 
void send_status_packet( uint8_t error, uint8_t *params, uint8_t param_lenght); 
void wait_return_delay(void); 

#endif
