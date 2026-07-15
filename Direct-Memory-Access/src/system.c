#include "system.h"
#include "stm32f103xb.h"

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
                                            // CR é o registrador de controle do RCC (Reset and Clock Control)
    RCC->CR |= RCC_CR_HSEON;                // HSEON - habilita o oscilador externo HSE                                                   
    while(!(RCC->CR & RCC_CR_HSERDY));      // Aguarda o bit HSERDY indicar que o clock externo está estável

    FLASH->ACR |= FLASH_ACR_LATENCY_2;      // ACR - registrador de controle da Flash, LATENCY_2 configura 2 ciclos de espera da memória Flash

    RCC->CFGR |=                            // CFGR - registrador de configuração do clock
          RCC_CFGR_HPRE_DIV1                // HPRE - prescaler do barramento AHB, DIV1 - mantém a frequência do clock AHB igual ao clock do sistema
        | RCC_CFGR_PPRE1_DIV2               // PPRE1 - prescaler do barramento APB1,  DIV2 - divide o clock APB1 por 2
        | RCC_CFGR_PPRE2_DIV1               // PPRE2 - prescaler do barramento APB2,  DIV1 - mantém o clock APB2 igual ao clock do sistema
        | RCC_CFGR_PLLSRC                   // PLLSRC - seleciona HSE como entrada do PLL (Phase-Locked Loop)
        | RCC_CFGR_PLLMULL9;                // PLLMULL9 - multiplica a frequência do PLL por 9

    RCC->CR |= RCC_CR_PLLON;                // PLLON - habilita o circuito PLL                                                    
    while(!(RCC->CR & RCC_CR_PLLRDY));      // Aguarda o bit PLLRDY indicar que o PLL está estabilizado
    RCC->CFGR |= RCC_CFGR_SW_PLL;           //  Define o PLL como fonte principal do clock (SYSCLK)
    
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  // Aguarda até o hardware confirmar que o PLL virou SYSCLK
                                                            // SWS informa qual clock está realmente sendo usada            

    SystemCoreClockUpdate();    // Atualiza a variável SystemCoreClock com a frequência atual
                                                       
}

// Inicialização dos pinos GPIO
static void gpio_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |= (0b1011 << 4);
}
/*
APB2ENR habilita o clock do periférico USART1 através do bit IOPAEN
CHR é o registrador de configuração dos pinos GPIO. Cada pino possui 4 bits de configuração.
~(0xF << 4) limpa os 4 bits de configuração do pino PA9
(0b1011 << 4) habilita os seguintes bits do PA9:
CNF1 CNF0 MODE1 MODE0
  1    0    1    1
*/


// Inicialização do USART1
static void usart1_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;      // Habilita o clock do periférico USART1 através do bit USART1EN
    USART1->BRR = SystemCoreClock / 115200;    // Configura a taxa de transmissão do USART1 para 115200 bps
    USART1->CR1 |= USART_CR1_TE;               // TE (Transmitter Enable) habilita o transmissor do USART1
    USART1->CR3 |= USART_CR3_DMAT;             // DMAT (DMA Enable Transmitter) habilita o uso do DMA para transmissão do USART1
    USART1->CR1 |= USART_CR1_UE;               // UE (USART Enable) habilita o USART1
}