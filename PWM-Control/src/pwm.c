#include "pwm.h"

// Inicializa o temporizador TIM2 no modo PWM:
void pwm_init(void) {
    // Habilita o clock do TIM2 e do GPIOA:
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Configura PA0 como Alternate Function (TIM2_CH1):
    GPIOA->MODER &= ~(0x3 << (0 * 2));
    GPIOA->MODER |= (0x2 << (0 * 2));
    GPIOA->AFR[0] &= ~(0xF << (0 * 4));
    GPIOA->AFR[0] |= (0x1 << (0 * 4));  // AF1 = TIM2

    // Configura o TIM2 para operar em modo PWM no canal 1:
    
    TIM2->CR1 = 0; // Reseta o registrador de controle do TIM2:

    /*
    Clock do TIM2 = APB1 = 90MHz (pois PLL = 180MHz e APB1 = 90MHz).
    Prescaler = 90-1 → 90MHz / 90 = 1MHz
    Período (ARR) = 999 → 1000 ciclos
    Frequência PWM = 1MHz / 1000 = 1kHz
    */
    TIM2->PSC = 90 - 1;
    TIM2->ARR = 999;

    // Configura o canal 1 para modo PWM 1 (saída ativa quando CNT < CCR):
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM2->CCMR1 |= (0x6 << TIM_CCMR1_OC1M_Pos);
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;    // Habilita pré-carga do registrador de compare

    // Habilita a saída do canal 1:
    TIM2->CCER |= TIM_CCER_CC1E;

    // Habilita a geração automática de atualização do registrador de compare:
    TIM2->CR1 |= TIM_CR1_ARPE;

    // Inicia o timer com duty cycle 0:
    pwm_set_duty_cycle(0);

    TIM2->CR1 |= TIM_CR1_CEN;
}
/*
AHB1ENR habilita o clock do periférico GPIOA através do bit GPIOAEN.
APB1ENR habilita o clock do TIM2 (APB1 = 90MHz).
PA0 configurado como AF1 (Alternate Function 1 = TIM2_CH1).
PSC = 90-1 → Clock do timer: 90MHz / 90 = 1MHz.
ARR = 999 → Período: 1000 ciclos → Frequência = 1MHz / 1000 = 1kHz.
CCMR1 OC1M = 0x6 → Modo PWM 1: canal ativo enquanto CNT < CCR.
ARPE habilita o buffer do registrador de auto-recarga (atualização segura).
*/

// Atualiza o duty cycle do PWM:
void pwm_set_duty_cycle(uint8_t duty) {
    // Limita o duty cycle entre 0 e 100:
    if(duty > 100) duty = 100;

    // Calcula o valor de compare e aplica ao registrador CCR1:
    TIM2->CCR1 = (TIM2->ARR * duty) / 100;
}