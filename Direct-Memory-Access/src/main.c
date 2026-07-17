#include "stm32f446xx.h"
#include "dma.h"
#include "system.h"

extern volatile uint8_t dma_transfer_complete;


uint8_t mensagem[] = "Teste DMA USART1\r\n";


int main(void){
    
    system_init();

    // Tranferência de dados do array "mensagem" para o registrador de dados do USART1 usando DMA
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

    while(1){

        if(dma_transfer_complete){
             //Transferência completa
            dma_transfer_complete = 0;
            GPIOA->ODR ^= (1 << 5); // Toggle LED PA5
        }

    }
}