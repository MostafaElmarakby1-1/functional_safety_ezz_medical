/*
 * UART_FRAME.c
 *
 *  Created on: Sep 7, 2026
 *      Author: User
 */


#include "main.h"
#include <stdint.h>
#include "UART_FRAME.h"
#include "stm32f4xx_hal_uart.h"



void Send_ACK(UART_HandleTypeDef *huart)
{
    uint8_t ack = ACK;
    HAL_UART_Transmit(huart, &ack, 1, 50);
}

void Send_NACK(UART_HandleTypeDef *huart)
{
    uint8_t nack = NACK;
    HAL_UART_Transmit(huart, &nack, 1, 50);
}


uint8_t CRC8(const uint8_t *data, uint8_t length)
{
    uint8_t crc = 0x00;

    for (uint8_t i = 0; i < length; i++)
    {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ 0x07;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}



void Send_Frame(USART_Frame_t *frame, UART_HandleTypeDef *huart)
{
    if (HAL_UART_Transmit_IT(huart,(uint8_t *)frame,sizeof(USART_Frame_t)) == HAL_OK)
    {
        uart_tx_complete = 0;
    }
}







