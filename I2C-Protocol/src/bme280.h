#ifndef BME280_H
#define BME280_H

#include <stdint.h>
#include "i2c_driver.h" 

/*
Driver para o sensor BME280 (temperatura, umidade e pressão).
Comunicação via I²C, utilizando o driver genérico I2C_*.
*/

typedef struct {
    int16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_H1;
    int16_t dig_H2;
    int16_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int16_t dig_H6;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    int32_t t_fine;
} BME280_Calibration;

// Inicializa o sensor BME280:
uint8_t BME280_Init(I2C_TypeDef *i2c, uint8_t addr);

// Lê a temperatura em graus Celsius:
float BME280_ReadTemperature(I2C_TypeDef *i2c, uint8_t addr);

// Lê a umidade relativa em porcentagem:
float BME280_ReadHumidity(I2C_TypeDef *i2c, uint8_t addr);

// Lê a pressão atmosférica em hPa:
float BME280_ReadPressure(I2C_TypeDef *i2c, uint8_t addr);

// Lê todos os dados (temp, umidade, pressão) de uma vez:
void BME280_ReadAll(I2C_TypeDef *i2c, uint8_t addr, float *temperature, float *humidity, float *pressure);

#endif