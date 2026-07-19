#include "uart_driver.h"
#include <string.h>

// Inicializa a UART com a velocidade de baud rate especificada:
void UART_Init(USART_TypeDef *uart, uint32_t baudrate) {
    // Habilita o clock da UART:
    if(uart == USART2) {
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    } else if(uart == USART1) {
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    } else if(uart == USART3) {
        RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    }

    // Configura os pinos GPIO para Alternate Function:
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // USART2: TX = PA2, RX = PA3
    // Configura PA2 e PA3 como Alternate Function:
    GPIOA->MODER &= ~(0x3 << 4); // Limpa bits do PA2
    GPIOA->MODER |= (0x2 << 4);  // Alternate Function para PA2
    GPIOA->MODER &= ~(0x3 << 6); // Limpa bits do PA3
    GPIOA->MODER |= (0x2 << 6);  // Alternate Function para PA3

    // Configura AF7 (USART2) para os pinos PA2 e PA3:
    GPIOA->AFR[0] &= ~(0xF << 8);  // Limpa AF do PA2
    GPIOA->AFR[0] |= (0x7 << 8);   // AF7 para PA2
    GPIOA->AFR[0] &= ~(0xF << 12); // Limpa AF do PA3
    GPIOA->AFR[0] |= (0x7 << 12);  // AF7 para PA3

    // Configura como Push-Pull para TX:
    GPIOA->OTYPER &= ~(0x1 << 2);
    GPIOA->OSPEEDR |= (0x3 << 4);
    GPIOA->OSPEEDR |= (0x3 << 6);
    GPIOA->PUPDR &= ~(0x3 << 4);
    GPIOA->PUPDR |= (0x1 << 4);

    // Desabilita a UART para configuração:
    uart->CR1 &= ~USART_CR1_UE;

    // Calcula o divisor de baud rate (PCLK = 45MHz para APB1):
    uint32_t pclk = 45000000;
    uint32_t usartdiv = (pclk * 16) / baudrate;
    uart->BRR = usartdiv;

    // Configura: 8 bits, 1 stop bit, sem paridade:
    uart->CR1 |= USART_CR1_TE;   // Habilita transmissão
    uart->CR1 |= USART_CR1_RE;   // Habilita recepção
    uart->CR2 &= ~USART_CR2_STOP; // 1 stop bit
    uart->CR1 &= ~USART_CR1_M;   // 8 bits de dados

    // Habilita a UART:
    uart->CR1 |= USART_CR1_UE;
}

// Aguarda até que o buffer de transmissão esteja vazio:
static inline void uart_wait_txe(USART_TypeDef *uart) {
    while(!(uart->SR & USART_SR_TXE));
}

// Aguarda até que a transmissão seja concluída:
static inline void uart_wait_tc(USART_TypeDef *uart) {
    while(!(uart->SR & USART_SR_TC));
}

// Envia um único caractere pela UART:
void UART_SendChar(USART_TypeDef *uart, char c) {
    uart_wait_txe(uart);
    uart->DR = c;
}

// Envia uma string pela UART:
void UART_SendString(USART_TypeDef *uart, const char *str) {
    while(*str) {
        UART_SendChar(uart, *str++);
    }
    uart_wait_tc(uart);
}