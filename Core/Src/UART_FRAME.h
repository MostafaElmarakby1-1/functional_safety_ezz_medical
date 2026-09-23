#ifndef UART_FRAME_H
#define UART_FRAME_H

#include <stdint.h>
#include "stm32f4xx_hal_uart.h"

#define ID_1 1
#define ID_2 2
#define ACK   0x06
#define NACK  0x15
#define SENDER_ID 10
typedef struct
{
    uint8_t id;
    uint8_t data[8];
    uint8_t liveness;
    uint8_t crc;
} USART_Frame_t;

extern volatile uint8_t uart_tx_complete;

uint8_t CRC8(const uint8_t *data, uint8_t length);

void Send_Frame(USART_Frame_t *frame, UART_HandleTypeDef *huart);

void Recive_Frame(USART_Frame_t *frame, UART_HandleTypeDef *huart);

void Frame(void);
void Send_ACK(UART_HandleTypeDef *huart);
void Send_NACK(UART_HandleTypeDef *huart);

#endif
