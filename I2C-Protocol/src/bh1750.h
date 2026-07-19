#ifndef BH1750_H
#define BH1750_H

#include <stdint.h>
#include "i2c_driver.h" 

/*
Driver para o sensor BH1750 (luminosidade ambiente).
Comunicação via I²C, utilizando o driver genérico I2C_*.
*/

// Modos de operação do BH1750:
#define BH1750_MODE_POWER_DOWN      0x00
#define BH1750_MODE_POWER_ON        0x01
#define BH1750_MODE_RESET           0x07
#define BH1750_MODE_CONT_H_RES      0x10  // Resolução alta contínua (1 lx)
#define BH1750_MODE_CONT_H_RES2     0x11  // Resolução alta contínua (0.5 lx)
#define BH1750_MODE_CONT_L_RES      0x13  // Resolução baixa contínua (4 lx)
#define BH1750_MODE_ONE_H_RES       0x20  // Resolução alta single-shot
#define BH1750_MODE_ONE_H_RES2      0x21  // Resolução alta single-shot (0.5 lx)
#define BH1750_MODE_ONE_L_RES       0x23  // Resolução baixa single-shot (4 lx)

// Inicializa o sensor BH1750:
uint8_t BH1750_Init(I2C_TypeDef *i2c, uint8_t addr);

// Configura o modo de operação do sensor:
void BH1750_SetMode(I2C_TypeDef *i2c, uint8_t addr, uint8_t mode);

// Lê a intensidade luminosa em lux:
float BH1750_ReadLux(I2C_TypeDef *i2c, uint8_t addr);

#endif