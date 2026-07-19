#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <stdint.h>
#include <stm32f4xx.h>

typedef struct {
    float temperature;
    float humidity;
    float pressure;
    float luminosity;
} SensorData;

// Inicializa todos os sensores do sistema:
void SensorManager_Init(I2C_TypeDef *i2c);

// Atualiza as leituras de todos os sensores:
void SensorManager_Update(void);

// Retorna os dados mais recentes do sistema:
SensorData SensorManager_GetData(void);

#endif