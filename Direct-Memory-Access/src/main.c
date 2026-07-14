#include "stm32f103xb.h"
#include "dma.h"
#include "system.h"

uint8_t mensagem[] = "Teste DMA USART1\r\n";

int main(void){

    system_init();

    dma_start_transfer(
        DMA1,
        DMA1_Channel4,
        4,
        mensagem,
        &USART1->DR,
        sizeof(mensagem)
    );


    while(1)
    {

    }
}