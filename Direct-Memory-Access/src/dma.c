#include "dma.h"

volatile uint8_t dma_transfer_complete = 0;

void dma_start_transfer(DMA_TypeDef *dma, DMA_Stream_TypeDef *stream, uint8_t stream_number, uint8_t channel, volatile void *src, volatile void *dst, uint16_t length, IRQn_Type irq) {

  if(dma == DMA1) RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;         // AHB1ENR é o registrador de habilitação do clock do barramento AHB1
  else if(dma == DMA2) RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;     // DMA1EN e DMA2EN são os bits de habilitação do clock do DMA1 e DMA2, respectivamente
  
  while(stream->CR & DMA_SxCR_EN);   // Aguarda liberação do bit Enable do DMA
  stream->CR &= ~DMA_SxCR_EN;        // Garante que o EN seja colocado em 0

  __DSB();      // Espera as instruções anteriores terminarem antes de prosseguir

  stream->CR &= ~DMA_SxCR_CHSEL;                      // Limpa o campo CHSEL
  stream->CR |= (channel << DMA_SxCR_CHSEL_Pos);      // CHSEL seleciona o canal do stream DMA
  stream->M0AR = (uint32_t)src;                       // M0AR - endereço de memória do DMA
  stream->PAR = (uint32_t)dst;                        // PAR - endereço do periférico do DMA
  stream->NDTR = length;                              // NDTR - número de dados do DMA

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

  NVIC_EnableIRQ(irq);  // Habilita a interrupção do DMA de acordo com o stream escolhido
  dma_transfer_complete = 0;
  stream->CR |= DMA_SxCR_EN; // Seta o enable stream do DMA
}

// Rotina de interrupção do stream utilizado para a transmissão de dados do USART1
void DMA2_Stream7_IRQHandler(void){
    if(DMA2->HISR & DMA_HISR_TCIF7){      // DMA_HISR_TCIF7 é a flag de transferência completa do stream 7 do DMA2
        DMA2->HIFCR |= DMA_HIFCR_CTCIF7;  // Limpa a flag de transferência completa
        dma_transfer_complete = 1;        // Seta a flag de transferência completa para ser utilizada na rotina principal
    }
}

// Rotina de interrupção do stream utilizado para aquisição ADC
void DMA2_Stream0_IRQHandler(void){
    if(DMA2->LISR & DMA_LISR_TCIF0){
        DMA2->LIFCR |= DMA_LIFCR_CTCIF0;
        dma_transfer_complete = 1;
    }
}

// TODO: deixar as rotinas de interrupção genéricas