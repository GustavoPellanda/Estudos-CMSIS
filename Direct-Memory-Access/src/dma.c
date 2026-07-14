#include "dma.h"

void dma_start_transfer(DMA_TypeDef *dma, DMA_Channel_TypeDef *channel, uint8_t channel_number, volatile void *src, volatile void *dst, uint16_t length) {

                                                      // CCR é o registrador de controle do DMA
    while(channel->CCR & DMA_CCR_EN);                 // Aguarda liberação do bit Enable do DMA
    channel->CCR &= ~DMA_CCR_EN;                      // Garante que o EN seja colocado em 0

    __DSB();                                          // Espera as instruções anteriores terminarem antes de prosseguir    

    channel->CMAR = (uint32_t)dst;                    // CMAR - endereço de memória do DMA
    channel->CPAR = (uint32_t)src;                    // CPAR - endereço do periférico do DMA
    channel->CNDTR = length;                          // CNDTR - número de dados do DMA
    
    uint32_t flag_shift = (channel_number - 1) * 4;   // Pula a quantidade de bits correspondente ao canal escolhido

    dma->IFCR =                                       // IFCR - registrador de flags do DMA
          (DMA_IFCR_CGIF1   << flag_shift)            // Limpa a flag de interrupção
        | (DMA_IFCR_CTCIF1  << flag_shift)            // Limpa a flag de transferência completa
        | (DMA_IFCR_CTEIF1  << flag_shift);           // Limpa a flag de erro de transferência

    __DMB();                                          // Mantém a ordem das instruções anteriores

    channel->CCR |=
          DMA_CCR_MINC                                // Habilita incremento de memória
        | DMA_CCR_DIR                                 // Define a direção da transferência (memória para periférico)
        | DMA_CCR_TCIE                                // Habilita interrupção de transferência completa
        | DMA_CCR_PL_1;                               // Define a prioridade do canal como alta

    channel->CCR |= DMA_CCR_EN;                       // Seta o EN do DMA para 1

}

// TODO: Deixar a direção de transferência do DMA configurável (memória para periférico ou periférico para memória)
// TODO: Implementar a função dma_stop_transfer() para parar a transferência do DMA