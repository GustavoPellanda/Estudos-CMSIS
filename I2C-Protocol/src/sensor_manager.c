#include "sensor_manager.h"
#include "bme280.h"
#include "bh1750.h"

// Endereços I²C dos sensores:
#define BME280_I2C_ADDR     0x76
#define BH1750_I2C_ADDR     0x23

static I2C_TypeDef *i2c_bus;
static SensorData data;

// Inicializa todos os sensores do sistema:
void SensorManager_Init(I2C_TypeDef *i2c) {
    i2c_bus = i2c;

    // Inicializa o BME280:
    if(!BME280_Init(i2c_bus, BME280_I2C_ADDR)) {
        data.temperature = -999.0f;
        data.humidity = -999.0f;
        data.pressure = -999.0f;
    }

    // Inicializa o BH1750:
    if(!BH1750_Init(i2c_bus, BH1750_I2C_ADDR)) {
        data.luminosity = -999.0f;
    }
}

// Atualiza as leituras de todos os sensores:
void SensorManager_Update(void) {
    float temp, hum, press;

    // Lê todos os dados do BME280 de uma só vez:
    BME280_ReadAll(i2c_bus, BME280_I2C_ADDR, &temp, &hum, &press);
    data.temperature = temp;
    data.humidity = hum;
    data.pressure = press;

    // Lê a luminosidade do BH1750:
    data.luminosity = BH1750_ReadLux(i2c_bus, BH1750_I2C_ADDR);
}

// Retorna os dados mais recentes do sistema:
SensorData SensorManager_GetData(void) {
    return data;
}