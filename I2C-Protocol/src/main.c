#include <stm32f4xx.h>
#include "i2c_driver.h"
#include "uart_driver.h"
#include "sensor_manager.h"
#include <stdio.h>

// Buffer para formatação das strings:
static char print_buffer[128];

// Inicializa o sistema de clock:
static void system_clock_init(void) {
    // Habilita o HSE (High Speed External) oscillator:
    RCC->CR |= RCC_CR_HSEON;
    while(!(RCC->CR & RCC_CR_HSERDY));

    // Configura o PLL para 180MHz (HSE = 8MHz):
    RCC->PLLCFGR = (8 << RCC_PLLCFGR_PLLM_Pos) |   // PLLM = 8
                   (225 << RCC_PLLCFGR_PLLN_Pos) |  // PLLN = 225
                   (0 << RCC_PLLCFGR_PLLP_Pos) |    // PLLP = 2 (180MHz)
                   (7 << RCC_PLLCFGR_PLLQ_Pos) |    // PLLQ = 7
                   RCC_PLLCFGR_PLLSRC_HSE;

    // Habilita o PLL:
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));

    // Configura o Flash para operação em alta velocidade:
    FLASH->ACR |= FLASH_ACR_LATENCY_5WS;
    FLASH->ACR |= FLASH_ACR_ICEN;
    FLASH->ACR |= FLASH_ACR_DCEN;
    FLASH->ACR |= FLASH_ACR_PRFTEN;

    // Configura prescalers para APB1 e APB2 (APB1 = 45MHz, APB2 = 90MHz):
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;   // AHB = SYSCLK = 180MHz
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;  // APB1 = 45MHz
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;  // APB2 = 90MHz

    // Muda o clock para PLL:
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}

// Imprime os dados do sensor via UART:
static void print_sensor_data(SensorData d) {
    // Cria o cabeçalho visual:
    UART_SendString(USART2, "\r\n-----------------------------\r\n");

    // Formata e envia cada valor:
    sprintf(print_buffer, "Temperature : %.2f °C\r\n", d.temperature);
    UART_SendString(USART2, print_buffer);

    sprintf(print_buffer, "Humidity    : %.2f %%\r\n", d.humidity);
    UART_SendString(USART2, print_buffer);

    sprintf(print_buffer, "Pressure    : %.2f hPa\r\n", d.pressure);
    UART_SendString(USART2, print_buffer);

    sprintf(print_buffer, "Luminosity  : %.2f lx\r\n", d.luminosity);
    UART_SendString(USART2, print_buffer);

    UART_SendString(USART2, "-----------------------------\r\n");
}

// Ponto de entrada do programa:
int main(void) {
    // Inicializa o sistema:
    system_clock_init();

    // Inicializa os periféricos:
    I2C_Init(I2C1, 100000);
    UART_Init(USART2, 115200);

    // Inicializa os sensores:
    SensorManager_Init(I2C1);

    UART_SendString(USART2, "\r\nInicializacao concluida!\r\n");

    while(1) {
        // Atualiza as leituras:
        SensorManager_Update();

        // Publica os dados pela UART:
        SensorData d = SensorManager_GetData();
        print_sensor_data(d);

        // Delay de 1 segundo (aproximado):
        for(volatile int i = 0; i < 5000000; i++);
    }
}