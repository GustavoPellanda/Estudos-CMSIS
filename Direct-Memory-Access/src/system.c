#include "system.h"
#include "stm32f446xx.h"

static void clock_init(void);
static void gpio_init(void);
static void usart1_init(void);

void system_init(void) {
    clock_init();
    gpio_init();
    usart1_init();
}

// Inicialização do clock do sistema
static void clock_init(void){
    RCC->CR |= RCC_CR_HSEON;                // HSEON - habilita o oscilador externo HSE
    while(!(RCC->CR & RCC_CR_HSERDY));      // Aguarda o bit HSERDY indicar que o clock externo está estável

    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY_5WS;    // ACR - registrador de controle da Flash, LATENCY_5WS configura 5 ciclos de espera para 180MHz

    RCC->CFGR |=
          RCC_CFGR_HPRE_DIV1      // HPRE - prescaler do barramento AHB, DIV1 - mantém a frequência do clock AHB igual ao clock do sistema
        | RCC_CFGR_PPRE1_DIV4     // PPRE1 - prescaler do barramento APB1, DIV4 - divide o clock APB1 por 4
        | RCC_CFGR_PPRE2_DIV2;    // PPRE2 - prescaler do barramento APB2, DIV2 - divide o clock APB2 por 2

    RCC->PLLCFGR =
          RCC_PLLCFGR_PLLSRC_HSE            // PLLSRC - seleciona HSE como entrada do PLL
        | (4 << RCC_PLLCFGR_PLLM_Pos)       // PLLM = 4 - divide HSE de 8MHz por 4 para obter 2MHz na entrada do PLL
        | (180 << RCC_PLLCFGR_PLLN_Pos)     // PLLN = 180 - multiplica 2MHz por 180 para obter 360MHz no VCO
        | (0 << RCC_PLLCFGR_PLLP_Pos)       // PLLP = 2 - divide 360MHz por 2 para obter 180MHz no SYSCLK
        | (15 << RCC_PLLCFGR_PLLQ_Pos);     // PLLQ = 15 - divide 360MHz por 15 para obter 24MHz no clock USB

    RCC->CR |= RCC_CR_PLLON;                // PLLON - habilita o circuito PLL
    while(!(RCC->CR & RCC_CR_PLLRDY));      // Aguarda o bit PLLRDY indicar que o PLL está estabilizado

    RCC->CFGR |= RCC_CFGR_SW_PLL;           // Define o PLL como fonte principal do clock (SYSCLK)
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  // Aguarda até o hardware confirmar que o PLL virou SYSCLK

    SystemCoreClockUpdate();       // Atualiza a variável SystemCoreClock com a frequência atual
}

// Inicialização dos pinos GPIO
static void gpio_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(0x3 << 18);
    GPIOA->MODER |= (0x2 << 18);
    GPIOA->AFR[1] &= ~(0xF << 4);
    GPIOA->AFR[1] |= (0x7 << 4);
}
/*
AHB1ENR habilita o clock do periférico GPIOA através do bit GPIOAEN
MODER é o registrador de modo dos pinos GPIO. Cada pino possui 2 bits de configuração.
~(0x3 << 18) limpa os 2 bits de configuração do pino PA9
(0x2 << 18) configura PA9 com os seguintes bits:
MODE1 MODE0
  1    0    (modo 2 = Alternate Function)
AFR[1] é o registrador de função alternativa dos pinos PA8-PA15. Cada pino possui 4 bits de configuração.
~(0xF << 4) limpa os 4 bits de função alternativa do pino PA9
(0x7 << 4) seleciona AF7 para o pino PA9:
AF3 AF2 AF1 AF0
 0   1   1   1   (AF7 = USART1_TX)
*/

// Inicialização do USART1
static void usart1_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;   // Habilita o clock do periférico USART1 através do bit USART1EN
    USART1->BRR = SystemCoreClock / 115200; // Configura a taxa de transmissão do USART1 para 115200 bps
    USART1->CR1 |= USART_CR1_TE;            // TE (Transmitter Enable) habilita o transmissor do USART1
    USART1->CR3 |= USART_CR3_DMAT;          // DMAT (DMA Enable Transmitter) habilita o uso do DMA para transmissão do USART1
    USART1->CR1 |= USART_CR1_UE;            // UE (USART Enable) habilita o USART1
}