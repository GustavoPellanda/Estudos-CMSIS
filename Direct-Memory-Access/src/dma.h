#ifndef DMA_H
#define DMA_H

#include "stm32f446xx.h"

/*  Driver para transferências DMA */

// Variável global para sinalizar conclusão da transferência:
extern volatile uint8_t dma_transfer_complete;

void dma_start_transfer(
    DMA_TypeDef *dma,               // Controlador DMA (DMA1 ou DMA2)
    DMA_Stream_TypeDef *stream,     // Endereço do stream DMA
    uint8_t stream_number,          // Número do stream DMA (0 a 7)
    uint8_t channel,                // Canal do stream DMA (0 a 7)
    volatile void *src,             // Endereço de origem dos dados
    volatile void *dst,             // Endereço de destino dos dados
    uint16_t length,                // Quantidade de transferências
    IRQn_Type irq                   // Número da interrupção do stream DMA
);

#endif

/*
 * O STM32F446 possui 2 controladores DMA, que são periféricos responsáveis pela
 * transferência direta de dados entre a memória e os periféricos. Eles permitem
 * que os dados sejam transferidos sem intervenção direta da CPU, permitindo que
 * ela execute outras instruções enquanto a transferência ocorre e receba uma
 * notificação ao final da operação por meio de uma interrupção.
 *
 * Cada controlador DMA possui 8 streams distintos, permitindo que ele realize
 * transferências entre diferentes periféricos. Cada stream possui seus próprios
 * registradores de configuração, enquanto as flags de status dos streams ficam
 * armazenadas em registradores compartilhados do DMA. Por isso, para acessar as
 * flags de um stream específico, é necessário aplicar um deslocamento de bits até
 * a posição correspondente daquele stream no registrador. As flags dos streams 0-3
 * ficam em LIFCR (Low), e as dos streams 4-7 em HIFCR (High).
 */