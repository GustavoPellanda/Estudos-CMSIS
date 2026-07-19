#include "i2c_driver.h"

// Inicializa o periférico I²C com a velocidade especificada:
void I2C_Init(I2C_TypeDef *i2c, uint32_t clock_speed_hz) {
    // Habilita o clock do periférico I²C:
    if(i2c == I2C1) RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    else if(i2c == I2C2) RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    else if(i2c == I2C3) RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;

    // Desabilita o periférico para configuração:
    i2c->CR1 &= ~I2C_CR1_PE;

    // Configura os pinos GPIO para Alternate Function (I2C):
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // Configura PB6 (SCL) e PB7 (SDA) como Alternate Function, Open-Drain:
    GPIOB->MODER &= ~(0x3 << 12); // Limpa bits do PB6
    GPIOB->MODER |= (0x2 << 12);  // Alternate Function para PB6
    GPIOB->MODER &= ~(0x3 << 14); // Limpa bits do PB7
    GPIOB->MODER |= (0x2 << 14);  // Alternate Function para PB7

    // Configura AF4 (I2C1) para os pinos PB6 e PB7:
    GPIOB->AFR[0] &= ~(0xF << 24); // Limpa AF do PB6
    GPIOB->AFR[0] |= (0x4 << 24);  // AF4 para PB6
    GPIOB->AFR[0] &= ~(0xF << 28); // Limpa AF do PB7
    GPIOB->AFR[0] |= (0x4 << 28);  // AF4 para PB7

    // Configura os pinos como Open-Drain com pull-up externo:
    GPIOB->OTYPER |= (0x3 << 6);   // Open-Drain para PB6 e PB7
    GPIOB->OSPEEDR |= (0x3 << 12); // High speed para PB6
    GPIOB->OSPEEDR |= (0x3 << 14); // High speed para PB7
    GPIOB->PUPDR &= ~(0x3 << 12);  // Sem pull-up interno
    GPIOB->PUPDR &= ~(0x3 << 14);

    // Calcula os parâmetros de temporização para 100kHz (APB1 = 45MHz):
    uint32_t pclk1 = 45000000;
    uint32_t freq = pclk1 / 1000000;
    i2c->CR2 = (freq & 0x3F);

    // Configuração para 100kHz:
    i2c->CCR = 225; // 45MHz / (2 * 225) = 100kHz
    i2c->TRISE = freq + 1;

    // Habilita o periférico:
    i2c->CR1 |= I2C_CR1_PE;
}

// Aguarda até que a flag de evento seja setada:
static inline void i2c_wait_event(I2C_TypeDef *i2c, uint32_t event) {
    while(!(i2c->SR1 & event));
}

// Aguarda até que a flag de status seja setada e a resposta seja válida:
static inline void i2c_wait_status(I2C_TypeDef *i2c, uint32_t status) {
    while(!(i2c->SR1 & status));
}

// Escreve um único byte em um registrador do dispositivo:
void I2C_WriteRegister(I2C_TypeDef *i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t data) {
    // Gera condição START:
    i2c->CR1 |= I2C_CR1_START;
    i2c_wait_event(i2c, I2C_SR1_SB);

    // Envia o endereço do dispositivo (escrita):
    i2c->DR = (dev_addr << 1) & 0xFE;
    i2c_wait_event(i2c, I2C_SR1_ADDR);
    (void)i2c->SR2; // Limpa a flag ADDR lendo SR2

    // Envia o endereço do registrador:
    i2c->DR = reg_addr;
    i2c_wait_event(i2c, I2C_SR1_TXE);

    // Envia o dado:
    i2c->DR = data;
    i2c_wait_event(i2c, I2C_SR1_TXE);

    // Gera condição STOP:
    i2c->CR1 |= I2C_CR1_STOP;
}

// Lê um único byte de um registrador do dispositivo:
uint8_t I2C_ReadRegister(I2C_TypeDef *i2c, uint8_t dev_addr, uint8_t reg_addr) {
    // Gera condição START:
    i2c->CR1 |= I2C_CR1_START;
    i2c_wait_event(i2c, I2C_SR1_SB);

    // Envia o endereço do dispositivo (escrita):
    i2c->DR = (dev_addr << 1) & 0xFE;
    i2c_wait_event(i2c, I2C_SR1_ADDR);
    (void)i2c->SR2; // Limpa a flag ADDR lendo SR2

    // Envia o endereço do registrador:
    i2c->DR = reg_addr;
    i2c_wait_event(i2c, I2C_SR1_TXE);

    // Gera condição START (repeated start):
    i2c->CR1 |= I2C_CR1_START;
    i2c_wait_event(i2c, I2C_SR1_SB);

    // Envia o endereço do dispositivo (leitura):
    i2c->DR = (dev_addr << 1) | 0x01;
    i2c_wait_event(i2c, I2C_SR1_ADDR);
    (void)i2c->SR2; // Limpa a flag ADDR lendo SR2

    // Desabilita ACK para leitura de um único byte:
    i2c->CR1 &= ~I2C_CR1_ACK;

    // Gera condição STOP:
    i2c->CR1 |= I2C_CR1_STOP;

    // Aguarda o dado recebido:
    i2c_wait_event(i2c, I2C_SR1_RXNE);

    uint8_t result = i2c->DR;

    // Reabilita ACK para próximas leituras:
    i2c->CR1 |= I2C_CR1_ACK;

    return result;
}

// Lê múltiplos bytes consecutivos a partir de um registrador:
void I2C_ReadRegisters(I2C_TypeDef *i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t *buffer, uint8_t length) {
    // Gera condição START:
    i2c->CR1 |= I2C_CR1_START;
    i2c_wait_event(i2c, I2C_SR1_SB);

    // Envia o endereço do dispositivo (escrita):
    i2c->DR = (dev_addr << 1) & 0xFE;
    i2c_wait_event(i2c, I2C_SR1_ADDR);
    (void)i2c->SR2; // Limpa a flag ADDR lendo SR2

    // Envia o endereço do registrador:
    i2c->DR = reg_addr;
    i2c_wait_event(i2c, I2C_SR1_TXE);

    // Gera condição START (repeated start):
    i2c->CR1 |= I2C_CR1_START;
    i2c_wait_event(i2c, I2C_SR1_SB);

    // Envia o endereço do dispositivo (leitura):
    i2c->DR = (dev_addr << 1) | 0x01;
    i2c_wait_event(i2c, I2C_SR1_ADDR);
    (void)i2c->SR2; // Limpa a flag ADDR lendo SR2

    // Habilita ACK para leitura múltipla:
    i2c->CR1 |= I2C_CR1_ACK;

    // Lê os bytes:
    for(uint8_t i = 0; i < length; i++) {
        if(i == length - 1) {
            // Último byte: desabilita ACK e gera STOP
            i2c->CR1 &= ~I2C_CR1_ACK;
            i2c->CR1 |= I2C_CR1_STOP;
        }

        i2c_wait_event(i2c, I2C_SR1_RXNE);
        buffer[i] = i2c->DR;
    }

    // Reabilita ACK para próximas leituras:
    i2c->CR1 |= I2C_CR1_ACK;
}

// Escreve múltiplos bytes consecutivos a partir de um registrador:
void I2C_WriteRegisters(I2C_TypeDef *i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t *buffer, uint8_t length) {
    // Gera condição START:
    i2c->CR1 |= I2C_CR1_START;
    i2c_wait_event(i2c, I2C_SR1_SB);

    // Envia o endereço do dispositivo (escrita):
    i2c->DR = (dev_addr << 1) & 0xFE;
    i2c_wait_event(i2c, I2C_SR1_ADDR);
    (void)i2c->SR2; // Limpa a flag ADDR lendo SR2

    // Envia o endereço do registrador:
    i2c->DR = reg_addr;
    i2c_wait_event(i2c, I2C_SR1_TXE);

    // Envia os dados:
    for(uint8_t i = 0; i < length; i++) {
        i2c->DR = buffer[i];
        i2c_wait_event(i2c, I2C_SR1_TXE);
    }

    // Gera condição STOP:
    i2c->CR1 |= I2C_CR1_STOP;
}