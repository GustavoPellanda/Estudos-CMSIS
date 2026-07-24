#ifndef PWM_H
#define PWM_H

#include "stm32f446xx.h"
#include <stdint.h>

/*
Gerador de sinais PWM utilizando o temporizador TIM2 do STM32F446.
Configura o timer para operar em modo PWM e permite controle do duty cycle.
*/

void pwm_init(void);
void pwm_set_duty_cycle(uint8_t duty);  // Duty de 0 a 100 (%)

#endif

/**
 * Para utilizar um pino como saída de um Timer (PWM), algumas etapas de
 * configuração são necessárias:
 *
 * 1. Habilitar o clock do GPIO correspondente, permitindo o acesso aos seus
 *    registradores de configuração.
 *
 * 2. Configurar o pino no modo Alternate Function (AF). Nesse modo, o pino
 *    deixa de ser controlado diretamente pelos registradores do GPIO e passa
 *    a ser controlado por um periférico interno do microcontrolador.
 *
 * 3. Selecionar, no registrador AFR (Alternate Function Register), qual
 *    periférico será conectado ao pino. Um mesmo pino pode oferecer diversas
 *    funções alternativas (Timer, USART, SPI, I2C, etc.), e o valor escrito
 *    em AFR determina qual delas será utilizada.
 *
 * 4. Habilitar o clock do Timer e configurá-lo para operar no modo PWM.
 *    Após essa configuração, o próprio hardware do Timer gera o sinal PWM,
 *    alternando automaticamente o estado lógico do pino de acordo com o
 *    período e o duty cycle programados, sem necessidade de intervenção
 *    contínua da CPU.
 */