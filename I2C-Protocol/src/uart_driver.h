#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include <stm32f4xx.h>

/*
Driver para comunicação UART utilizando CMSIS.
Responsável pela inicialização e envio de dados pela porta serial.
*/

void UART_Init(USART_TypeDef *uart, uint32_t baudrate);

// Envia uma string pela UART (bloqueante):
void UART_SendString(USART_TypeDef *uart, const char *str);

// Envia um único caractere pela UART (bloqueante):
void UART_SendChar(USART_TypeDef *uart, char c);

#endif