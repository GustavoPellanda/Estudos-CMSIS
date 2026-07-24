#include "stm32f446xx.h"
#include "pwm.h"
#include "motor.h"
#include <stdint.h>
#include <stdlib.h>

static void clock_init(void);
static void gpio_init(void);
static void usart_init(void);
static void usart_send_string(char *string);
static uint8_t usart_receive_speed(void);

int main(void) {
    clock_init();
    gpio_init();
    usart_init();
    pwm_init();
    //motor_init();

    usart_send_string("Controle de motor PWM iniciado\r\n");
    usart_send_string("Digite a velocidade (0-100):\r\n");

    while(1) {
        uint8_t speed = usart_receive_speed();

        if(speed <= 100) {
            //motor_set_speed(speed);
            usart_send_string("Velocidade atualizada\r\n");
        }
        else {
            usart_send_string("Valor invalido\r\n");
        }
    }
}

// Configura o clock principal do microcontrolador:
static void clock_init(void) {
    // Habilita HSE:
    RCC->CR |= RCC_CR_HSEON;
    while(!(RCC->CR & RCC_CR_HSERDY));

    // Configura Flash para operação em alta frequência:
    FLASH->ACR |= FLASH_ACR_LATENCY_5WS;

    // Configura PLL:
    RCC->PLLCFGR = (8 << RCC_PLLCFGR_PLLM_Pos) |
                   (360 << RCC_PLLCFGR_PLLN_Pos) |
                   (0 << RCC_PLLCFGR_PLLP_Pos);

    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));

    // Seleciona PLL como clock do sistema:
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}
/*
HSEON habilita o oscilador externo de alta velocidade (HSE).
PLLM = 8 → divide o HSE (8MHz) por 8 = 1MHz.
PLLN = 360 → multiplica por 360 = 360MHz.
PLLP = 0 → divide por 2 = 180MHz (máximo para STM32F446).
Flash Latency 5WS necessária para operação acima de 150MHz.
*/

// Configura os pinos utilizados pela aplicação:
static void gpio_init(void) {
    // Habilita clock do GPIOA:
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Configura PA5 como saída para teste (LED da Nucleo):
    GPIOA->MODER &= ~(0x3 << (5 * 2));
    GPIOA->MODER |= (0x1 << (5 * 2));
}

// Configura USART para comunicação serial:
static void usart_init(void) {
    // Habilita clocks:
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Configura PA9 como Alternate Function USART1_TX:
    GPIOA->MODER &= ~(0x3 << (9 * 2));
    GPIOA->MODER |= (0x2 << (9 * 2));
    GPIOA->AFR[1] |= (7 << ((9 - 8) * 4));

    // Configura PA10 como Alternate Function USART1_RX:
    GPIOA->MODER &= ~(0x3 << (10 * 2));
    GPIOA->MODER |= (0x2 << (10 * 2));
    GPIOA->AFR[1] |= (7 << ((10 - 8) * 4));

    // Configura baudrate 115200 considerando APB2 = 90MHz:
    USART1->BRR = 90000000 / 115200;

    // Habilita transmissor, receptor e USART:
    USART1->CR1 |= USART_CR1_TE;
    USART1->CR1 |= USART_CR1_RE;
    USART1->CR1 |= USART_CR1_UE;
}
/*
AFR[1] controla os pinos PA8 a PA15 (AF7 = USART1 para PA9/PA10).
APB2 opera a 90MHz (180MHz/2) para STM32F446 com PLL configurado.
BRR = 90000000/115200 ≈ 781 → Baudrate real de aproximadamente 115200.
TE habilita o transmissor, RE habilita o receptor, UE habilita o USART.
*/

// Envia uma string pela USART:
static void usart_send_string(char *string) {
    while(*string) {
        while(!(USART1->SR & USART_SR_TXE));
        USART1->DR = *string++;
    }
}

// Recebe o valor de velocidade enviado pelo usuário via serial:
static uint8_t usart_receive_speed(void) {
    char buffer[4];
    uint8_t index = 0;

    // Lê caracteres até encontrar fim de linha ou buffer cheio:
    while(index < sizeof(buffer) - 1) {
        while(!(USART1->SR & USART_SR_RXNE));               // Aguarda dado disponível
        char character = USART1->DR;                        // Lê caractere do registrador
        if(character == '\r' || character == '\n') break; 
        buffer[index++] = character; 
    }

    buffer[index] = '\0';    // Finaliza string
    return atoi(buffer);     // Converte para inteiro
}