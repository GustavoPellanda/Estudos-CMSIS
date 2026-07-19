#include "stm32f446xx.h"
#include "system.h"
#include "dma.h"
#include "adc.h"

# define ADC_SAMPLE_TIME_480CYCLES 0b111 // Define o sample time do ADC como 480 ciclos de clock

extern volatile uint8_t dma_transfer_complete;

uint8_t mensagem[] = "Teste DMA USART1\r\n";
uint16_t adc_buffer[128];

int main(void){
    
    system_init();
    adc_init(0, ADC_SAMPLE_TIME_480CYCLES); // Inicializa o ADC1 no canal 0

    // Tranferência de dados do array "mensagem" para o registrador de dados do USART1 usando DMA:
    dma_start_transfer(
        DMA2,
        DMA2_Stream7,
        7,  // Stream memória -> USART
        4, // Canal do USART1_TX
        mensagem,
        &USART1->DR,
        sizeof(mensagem),
        DMA2_Stream7_IRQn
    );

    // Tranferência de dados do registrador de dados do ADC1 para o array "adc_buffer" usando DMA:
    dma_start_transfer(
        DMA2,
        DMA2_Stream0,
        0, // Stream ADC -> memória
        0, // Canal do ADC1
        &ADC1->DR,
        adc_buffer,
        128,
        DMA2_Stream0_IRQn
    );

    adc_start();

    while(1){

        if(dma_transfer_complete){
             //Transferência completa
            dma_transfer_complete = 0;
            GPIOA->ODR ^= (1 << 5); // Toggle LED PA5
        }

    }
}

// TODO: Corrigir a flag para suportar múltiplos streams.
// TODO: Habilitar ADC contínuo.
// TODO: Habilitar DMA circular.
// TODO: Processar adc_buffer enquanto o DMA continua adquirindo.