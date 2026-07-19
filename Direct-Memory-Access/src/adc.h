#ifndef ADC_H
#define ADC_H

#include "stm32f446xx.h"


/*
    Inicializa ADC1 utilizando o canal informado
*/
void adc_init(uint8_t channel, uint8_t sample_time);


/*
    Inicia uma sequência de conversão
*/
void adc_start(void);


#endif