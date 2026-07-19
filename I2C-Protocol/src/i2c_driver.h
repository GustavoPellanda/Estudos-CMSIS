#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <stm32f4xx.h>

/*
Driver genérico de comunicação I²C.
Gerencia a inicialização do periférico e operações de leitura/escrita
no barramento, abstraindo detalhes de registradores.
*/

void I2C_Init(I2C_TypeDef *i2c, uint32_t clock_speed_hz);

// Escreve um único byte em um registrador do dispositivo:
void I2C_WriteRegister(
    I2C_TypeDef *i2c,           // Periférico I²C (I2C1, I2C2, etc.)
    uint8_t dev_addr,           // Endereço do dispositivo (7 bits)
    uint8_t reg_addr,           // Endereço do registrador a ser escrito
    uint8_t data                // Dado a ser escrito
);

// Lê um único byte de um registrador do dispositivo:
uint8_t I2C_ReadRegister(
    I2C_TypeDef *i2c,           // Periférico I²C (I2C1, I2C2, etc.)
    uint8_t dev_addr,           // Endereço do dispositivo (7 bits)
    uint8_t reg_addr            // Endereço do registrador a ser lido
);

// Lê múltiplos bytes consecutivos a partir de um registrador:
void I2C_ReadRegisters(
    I2C_TypeDef *i2c,           // Periférico I²C (I2C1, I2C2, etc.)
    uint8_t dev_addr,           // Endereço do dispositivo (7 bits)
    uint8_t reg_addr,           // Endereço inicial do registrador
    uint8_t *buffer,            // Buffer para armazenar os dados lidos
    uint8_t length              // Número de bytes a serem lidos
);

// Escreve múltiplos bytes consecutivos a partir de um registrador:
void I2C_WriteRegisters(
    I2C_TypeDef *i2c,           // Periférico I²C (I2C1, I2C2, etc.)
    uint8_t dev_addr,           // Endereço do dispositivo (7 bits)
    uint8_t reg_addr,           // Endereço inicial do registrador
    uint8_t *buffer,            // Buffer com os dados a serem escritos
    uint8_t length              // Número de bytes a serem escritos
);

#endif