#ifndef DMA_H
#define DMA_H

#include "stm32f103xb.h"

/*  Driver para transferências DMA */

void dma_start_transfer(
    DMA_TypeDef *dma,             // Controlador DMA (DMA1 ou DMA2)
    DMA_Channel_TypeDef *channel, // Endereço do canal DMA
    uint8_t channel_number,       // Número do canal DMA (1 a 7)
    volatile void *src,           // Endereço de origem dos dados
    volatile void *dst,           // Endereço de destino dos dados
    uint16_t length               // Quantidade de transferências
);

#endif

/*
 * O STM32F103 possui 2 controladores DMA, que são periféricos responsáveis pela
 * transferência direta de dados entre a memória e os periféricos. Eles permitem
 * que os dados sejam transferidos sem intervenção direta da CPU, permitindo que
 * ela execute outras instruções enquanto a transferência ocorre e receba uma
 * notificação ao final da operação por meio de uma interrupção.
 *
 * O mesmo DMA pode possuir vários canais distintos, permitindo que ele realize
 * transferências entre diferentes periféricos. Cada canal possui seus próprios
 * registradores de configuração, enquanto as flags de status dos canais ficam
 * armazenadas em registradores compartilhados do DMA. Por isso, para acessar as
 * flags de um canal específico, é necessário aplicar um deslocamento de bits até
 * a posição correspondente daquele canal no registrador.
 */