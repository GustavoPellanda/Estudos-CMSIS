#include "bh1750.h"
#include "i2c_driver.h"

// Endereço padrão do BH1750 (ADDR = LOW): 0x23
#define BH1750_DEFAULT_ADDR    0x23

// Inicializa o sensor BH1750:
uint8_t BH1750_Init(I2C_TypeDef *i2c, uint8_t addr) {
    // Liga o sensor:
    I2C_WriteRegister(i2c, addr, BH1750_MODE_POWER_ON, 0);

    // Aguarda estabilização:
    for(volatile int i = 0; i < 1000; i++);

    // Configura o modo de alta resolução contínua:
    BH1750_SetMode(i2c, addr, BH1750_MODE_CONT_H_RES);

    return 1;
}

// Configura o modo de operação do sensor:
void BH1750_SetMode(I2C_TypeDef *i2c, uint8_t addr, uint8_t mode) {
    // O BH1750 não usa registradores: o comando é enviado diretamente como se fosse o "endereço do registrador".
    i2c->CR1 |= I2C_CR1_START;
    while(!(i2c->SR1 & I2C_SR1_SB));

    i2c->DR = (addr << 1) & 0xFE;
    while(!(i2c->SR1 & I2C_SR1_ADDR));
    (void)i2c->SR2;

    i2c->DR = mode;
    while(!(i2c->SR1 & I2C_SR1_TXE));

    i2c->CR1 |= I2C_CR1_STOP;
}

// Lê a intensidade luminosa em lux:
float BH1750_ReadLux(I2C_TypeDef *i2c, uint8_t addr) {
    uint8_t buffer[2];

    // Lê 2 bytes do sensor (o BH1750 não usa registrador de endereço):
    i2c->CR1 |= I2C_CR1_START;
    while(!(i2c->SR1 & I2C_SR1_SB));

    i2c->DR = (addr << 1) | 0x01;
    while(!(i2c->SR1 & I2C_SR1_ADDR));
    (void)i2c->SR2;

    // Habilita ACK para leitura de 2 bytes:
    i2c->CR1 |= I2C_CR1_ACK;

    // Lê o primeiro byte com ACK:
    while(!(i2c->SR1 & I2C_SR1_RXNE));
    buffer[0] = i2c->DR;

    // Lê o segundo byte com NACK e STOP:
    i2c->CR1 &= ~I2C_CR1_ACK;
    i2c->CR1 |= I2C_CR1_STOP;

    while(!(i2c->SR1 & I2C_SR1_RXNE));
    buffer[1] = i2c->DR;

    // Reabilita ACK para próximas leituras:
    i2c->CR1 |= I2C_CR1_ACK;

    // Converte para lux (resolução de 1 lx):
    uint16_t raw = (buffer[0] << 8) | buffer[1];
    float lux = (float)raw / 1.2f;

    return lux;
}