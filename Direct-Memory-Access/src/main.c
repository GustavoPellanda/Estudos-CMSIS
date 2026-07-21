#include <stdio.h>
#include "stm32f446xx.h"
#include "system.h"
#include "dma.h"
#include "adc.h"

/*
 * Projeto: Driver Genérico de DMA com Pipeline ADC → Processamento → USART
 *
 * Objetivo:
 * Desenvolver e validar um driver genérico de DMA para o STM32F446,
 * explorando transferências entre periféricos e memória sem intervenção
 * constante da CPU.
 *
 * O projeto utiliza um pipeline de aquisição e processamento de sinais como
 * aplicação prática para demonstrar o funcionamento do DMA:
 *
 * 1. ADC realiza leituras contínuas de um sinal analógico
 * 2. DMA transfere automaticamente os dados do ADC para buffers em memória
 * 3. CPU processa os blocos recebidos, removendo outliers e calculando
 *    estatísticas como média, mínimo e máximo
 * 4. USART transmite os resultados processados para um terminal serial
 *    utilizando DMA novamente
 *
 * Resultado:
 * O terminal serial exibe continuamente estatísticas das leituras do ADC,
 * demonstrando uma aplicação prática de transferências DMA paralelas e a
 * redução da carga de processamento da CPU.
 */

#define ADC_SAMPLE_TIME_480CYCLES 0b111 // Define o sample time do ADC como 480 ciclos de clock

// Criação dos buffers duplos de dados para o ADC e USART:
#define ADC_BUFFER_SIZE 64
#define USART_BUFFER_SIZE 256
uint16_t adc_buffer[2][ADC_BUFFER_SIZE];
uint8_t usart_buffer[2][USART_BUFFER_SIZE];

// Índices de sincronização:
volatile uint8_t adc_write_idx = 0;      // Qual buffer o DMA ADC está preenchendo (0 ou 1)
volatile uint8_t usart_write_idx = 0;    // Qual buffer a CPU está preenchendo (0 ou 1)

// Variáveis de controle de pipeline:
uint8_t adc_last_read = 0;      // Último índice lido pela CPU
uint8_t usart_last_sent = 0;    // Último índice enviado pelo USART

uint8_t mensagem[] = "Teste de DMA iniciado. O sistema será inicializado e começará a transmitir leituras do ADC.\r\n";

// Processamento do buffer ADC - remove outliers e formata para envio USART:
int process_and_format_adc(uint16_t *adc_data, int adc_len,uint8_t *output, int max_output){
    uint32_t sum = 0;
    uint16_t min = 0xFFFF, max = 0;
    
    // Calcula máximo, mínimo, e média:
    for(int i = 0; i < adc_len; i++){
        sum += adc_data[i];
        if(adc_data[i] < min) min = adc_data[i];
        if(adc_data[i] > max) max = adc_data[i];
    }
    uint16_t avg = sum / adc_len;
    
    // Formata para string:
    int len = snprintf(
        (char*)output, max_output,
        "AVG=%u MIN=%u MAX=%u\r\n",
        avg, min, max
    );
    return len;
}

int main(void){
    
    system_init();

    // Tranferência de dados do array "mensagem" para o registrador de dados do USART1 usando DMA:
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
    
    adc_init(0, ADC_SAMPLE_TIME_480CYCLES); // Inicializa o ADC1 no canal 

    // Inicia primeira transferência DMA ADC (preenche o buffer do adc buffer 0):
    dma_start_transfer(
        DMA2,
        DMA2_Stream0,
        0, // Stream ADC -> memória
        0, // Canal do ADC1
        &ADC1->DR,
        adc_buffer[adc_write_idx],
        ADC_BUFFER_SIZE,
        DMA2_Stream0_IRQn
    );

    adc_start(); // Dispara primeira conversão

    while(1){
        // Verifica se há novo buffer ADC pronto para processamento:
        if(adc_write_idx != adc_last_read){
            
            // Seleciona qual dos dois buffers será utilizado nesta iteração do pipeline:
            uint16_t *adc_input = adc_buffer[adc_last_read];
            uint8_t *usart_output = usart_buffer[usart_write_idx];
            
            // Processa dados: remove outliers, formata para string
            int len = process_and_format_adc(
                adc_input, ADC_BUFFER_SIZE,
                usart_output, USART_BUFFER_SIZE
            );
            
            // Se USART não está transmitindo, inicia nova transferência:
            if(usart_write_idx == usart_last_sent){
                // USART está ocioso - dispara transmissão:
                dma_start_transfer(
                    DMA2, 
                    DMA2_Stream7, 
                    7, 
                    4,
                    usart_buffer[usart_write_idx],
                    &USART1->DR,
                    len,
                    DMA2_Stream7_IRQn
                );
                // Marca este buffer como enviado e alterna índice de escrita para o próximo buffer:
                usart_last_sent = usart_write_idx;
                usart_write_idx = (usart_write_idx == 0) ? 1 : 0;
            } else {
                // USART ainda transmitindo - apenas alterna índice de escrita:
                usart_write_idx = (usart_write_idx == 0) ? 1 : 0;
            }
            
            adc_last_read = adc_write_idx; // Marca este buffer como lido
            GPIOA->ODR ^= (1 << 5);  // LED pisca a cada novo ciclo
        }
        
        // cpu_work_counter++;
    }
}

// Ideia futura: organizar as info das transferências DMA em uma struct 
// Ideia futura: usar o cpu_work_counter para verificar quantos ciclos a CPU executou a cada recebimento de valores no terminal serial
// TODO: implementar remoção de outliers no processamento do buffer ADC