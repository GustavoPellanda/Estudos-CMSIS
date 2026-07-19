#include "adc.h"

void adc_init(uint8_t channel, uint8_t sample_time) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;     // Habilita o clock do GPIOA
    GPIOA->MODER &= ~(0b11 << 0);            // Limpa os dois bits do MODER, que configuram o pino PA0
    GPIOA->MODER |= (0b11 << 0);             // Seta os dois bits do MODER para 11, que correspondem ao modo analógico do pino PA0
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;      // Habilita o clock do ADC1

    ADC1->CR2 &= ~ADC_CR2_ADON;              // Desliga o ADC1 para configurar os registradores
    ADC1->SQR3 = channel;                    // SQR3 é o registrador de seleção de canais do ADC
    
    ADC1->SMPR2 &= ~(0x7 << (channel * 3));             // Limpa os bits de sample time do canal selecionado (cada canal possui 3 bits de configuração de sample time)
    ADC1->SMPR2 |= (sample_time << (channel * 3));      // Configura o sample time do canal selecionado (0 a 7) no registrador SMPR2
    
    ADC1->CR2 |=                                
          ADC_CR2_DMA                        // Habilita o DMA para o ADC
        | ADC_CR2_DDS;                       // Habilita o modo de transferência contínua de dados do ADC para o DMA
    ADC1->CR2 |= ADC_CR2_ADON;               // Seta o bit ADON para ligar o ADC1
}

void adc_start(void) {
    ADC1->CR2 |= ADC_CR2_SWSTART;   // Seta o bit SWSTART para iniciar a conversão do ADC
}