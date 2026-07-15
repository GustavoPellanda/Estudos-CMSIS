#include "dma.h"

void dma_start_transfer(DMA_TypeDef *dma, DMA_Stream_TypeDef *stream, uint8_t stream_number, volatile void *src, volatile void *dst, uint16_t length) {

  while(stream->CR & DMA_SxCR_EN);   // Aguarda liberação do bit Enable do DMA
  stream->CR &= ~DMA_SxCR_EN;        // Garante que o EN seja colocado em 0

  __DSB();                           // Espera as instruções anteriores terminarem antes de prosseguir

  stream->M0AR = (uint32_t)src;      // M0AR - endereço de memória do DMA
  stream->PAR = (uint32_t)dst;       // PAR - endereço do periférico do DMA
  stream->NDTR = length;             // NDTR - número de dados do DMA

  uint32_t flag_shift = (stream_number % 4) * 6;  // Pula a quantidade de bits correspondente ao stream escolhido

  if(stream_number < 4) {
    dma->LIFCR =
        (DMA_LIFCR_CFEIF0   << flag_shift)    // Limpa a flag de erro de FIFO
      | (DMA_LIFCR_CDMEIF0  << flag_shift)    // Limpa a flag de erro de modo direto
      | (DMA_LIFCR_CTEIF0   << flag_shift)    // Limpa a flag de erro de transferência
      | (DMA_LIFCR_CHTIF0   << flag_shift)    // Limpa a flag de meia transferência
      | (DMA_LIFCR_CTCIF0   << flag_shift);   // Limpa a flag de transferência completa
  } else {
      dma->HIFCR =
        (DMA_HIFCR_CFEIF4   << flag_shift)
      | (DMA_HIFCR_CDMEIF4  << flag_shift)
      | (DMA_HIFCR_CTEIF4   << flag_shift)
      | (DMA_HIFCR_CHTIF4   << flag_shift)
      | (DMA_HIFCR_CTCIF4   << flag_shift);
  }

  __DMB();    // Mantém a ordem das instruções anteriores

  stream->CR |=
      DMA_SxCR_MINC     // Habilita incremento de memória
    | DMA_SxCR_DIR_0    // Define a direção da transferência (memória para periférico)
    | DMA_SxCR_TCIE     // Habilita interrupção de transferência completa
    | DMA_SxCR_PL_1;    // Define a prioridade do stream como alta

  stream->CR |= DMA_SxCR_EN;    // Seta o EN do DMA para 1
}

// TODO: Deixar a direção de transferência do DMA configurável (memória para periférico ou periférico para memória)
// TODO: Implementar a função dma_stop_transfer() para parar a transferência do DMA