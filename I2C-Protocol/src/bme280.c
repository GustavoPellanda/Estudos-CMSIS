#include "bme280.h"
#include "i2c_driver.h"

// Endereços dos registradores do BME280:
#define BME280_REG_ID           0xD0
#define BME280_REG_RESET        0xE0
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_TEMP_MSB     0xFA
#define BME280_REG_PRESS_MSB    0xF7
#define BME280_REG_HUM_MSB      0xFD

#define BME280_CHIP_ID          0x60

static BME280_Calibration cal;

// Lê os parâmetros de calibração do sensor:
static void bme280_read_calibration(I2C_TypeDef *i2c, uint8_t addr) {
    uint8_t buffer[24];

    // Lê os parâmetros de calibração (0x88 a 0x9F):
    I2C_ReadRegisters(i2c, addr, 0x88, buffer, 24);

    cal.dig_T1 = (int16_t)(buffer[0] | (buffer[1] << 8));
    cal.dig_T2 = (int16_t)(buffer[2] | (buffer[3] << 8));
    cal.dig_T3 = (int16_t)(buffer[4] | (buffer[5] << 8));

    cal.dig_P1 = (uint16_t)(buffer[6] | (buffer[7] << 8));
    cal.dig_P2 = (int16_t)(buffer[8] | (buffer[9] << 8));
    cal.dig_P3 = (int16_t)(buffer[10] | (buffer[11] << 8));
    cal.dig_P4 = (int16_t)(buffer[12] | (buffer[13] << 8));
    cal.dig_P5 = (int16_t)(buffer[14] | (buffer[15] << 8));
    cal.dig_P6 = (int16_t)(buffer[16] | (buffer[17] << 8));
    cal.dig_P7 = (int16_t)(buffer[18] | (buffer[19] << 8));
    cal.dig_P8 = (int16_t)(buffer[20] | (buffer[21] << 8));
    cal.dig_P9 = (int16_t)(buffer[22] | (buffer[23] << 8));

    // Lê os parâmetros de calibração da umidade (0xA1 e 0xE1 a 0xE7):
    cal.dig_H1 = I2C_ReadRegister(i2c, addr, 0xA1);

    I2C_ReadRegisters(i2c, addr, 0xE1, buffer, 7);
    cal.dig_H2 = (int16_t)(buffer[0] | (buffer[1] << 8));
    cal.dig_H3 = (int16_t)(buffer[2]);
    cal.dig_H4 = (int16_t)((buffer[3] << 4) | (buffer[4] & 0x0F));
    cal.dig_H5 = (int16_t)((buffer[5] << 4) | (buffer[4] >> 4));
    cal.dig_H6 = (int16_t)(buffer[6]);
}

// Compensa a temperatura a partir dos dados brutos:
static int32_t bme280_compensate_temperature(int32_t adc_T) {
    int32_t var1 = (((adc_T >> 3) - ((int32_t)cal.dig_T1 << 1)) *
                    ((int32_t)cal.dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)cal.dig_T1)) *
                    ((adc_T >> 4) - ((int32_t)cal.dig_T1))) >> 12) *
                    ((int32_t)cal.dig_T3)) >> 14;
    cal.t_fine = var1 + var2;
    return (cal.t_fine * 5 + 128) >> 8;
}

// Compensa a umidade a partir dos dados brutos:
static int32_t bme280_compensate_humidity(int32_t adc_H) {
    int32_t var1 = cal.t_fine - ((int32_t)76800);
    int32_t var2 = (((int32_t)adc_H << 14) - (((int32_t)cal.dig_H4) << 20) -
                    (((int32_t)cal.dig_H5) * var1)) + ((int32_t)16384);
    
    int32_t var3 = (((((int32_t)adc_H << 14) - (((int32_t)cal.dig_H4) << 20) -
                    (((int32_t)cal.dig_H5) * var1)) + ((int32_t)16384)) >> 15);
    
    int32_t var4 = ((((((var3) * ((int32_t)cal.dig_H6)) >> 10) *
                    ((int32_t)cal.dig_H3)) >> 14) + (((int32_t)cal.dig_H2 * var3) >> 12));
    
    int32_t var5 = ((var4 >> 10) * ((int32_t)cal.dig_H6)) >> 10;
    return (var5 >> 1) * 262144 >> 12;
}

// Compensa a pressão a partir dos dados brutos:
static uint32_t bme280_compensate_pressure(int32_t adc_P) {
    int64_t var1 = ((int64_t)cal.t_fine) - 128000;
    int64_t var2 = var1 * var1 * (int64_t)cal.dig_P6;
    var2 = var2 + ((var1 * (int64_t)cal.dig_P5) << 17);
    var2 = var2 + (((int64_t)cal.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)cal.dig_P3) >> 8) +
           ((var1 * (int64_t)cal.dig_P2) << 12);
    var1 = (((((int64_t)1 << 47) + var1)) * ((int64_t)cal.dig_P1)) >> 33;

    if(var1 == 0) return 0;

    int64_t p = 1048576 - (int64_t)adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)cal.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)cal.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)cal.dig_P7) << 4);
    return (uint32_t)p;
}

// Inicializa o sensor BME280:
uint8_t BME280_Init(I2C_TypeDef *i2c, uint8_t addr) {
    // Verifica o chip ID:
    uint8_t id = I2C_ReadRegister(i2c, addr, BME280_REG_ID);
    if(id != BME280_CHIP_ID) return 0;

    // Faz o soft reset do sensor:
    I2C_WriteRegister(i2c, addr, BME280_REG_RESET, 0xB6);

    // Aguarda o reset completar (pelo menos 2ms):
    for(volatile int i = 0; i < 10000; i++);

    // Lê os parâmetros de calibração:
    bme280_read_calibration(i2c, addr);

    // Configura o sensor para modo normal:
    // Oversampling: temperatura x1, pressão x1, umidade x1
    I2C_WriteRegister(i2c, addr, BME280_REG_CTRL_HUM, 0x01);
    I2C_WriteRegister(i2c, addr, BME280_REG_CTRL_MEAS, 0x27);
    I2C_WriteRegister(i2c, addr, BME280_REG_CONFIG, 0x00);

    return 1;
}

// Lê a temperatura em graus Celsius:
float BME280_ReadTemperature(I2C_TypeDef *i2c, uint8_t addr) {
    uint8_t buffer[3];
    I2C_ReadRegisters(i2c, addr, BME280_REG_TEMP_MSB, buffer, 3);

    int32_t adc_T = (buffer[0] << 12) | (buffer[1] << 4) | (buffer[2] >> 4);
    int32_t temp = bme280_compensate_temperature(adc_T);
    return (float)temp / 100.0f;
}

// Lê a umidade relativa em porcentagem:
float BME280_ReadHumidity(I2C_TypeDef *i2c, uint8_t addr) {
    uint8_t buffer[2];
    I2C_ReadRegisters(i2c, addr, BME280_REG_HUM_MSB, buffer, 2);

    int32_t adc_H = (buffer[0] << 8) | buffer[1];
    int32_t hum = bme280_compensate_humidity(adc_H);
    return (float)hum / 1024.0f;
}

// Lê a pressão atmosférica em hPa:
float BME280_ReadPressure(I2C_TypeDef *i2c, uint8_t addr) {
    uint8_t buffer[3];
    I2C_ReadRegisters(i2c, addr, BME280_REG_PRESS_MSB, buffer, 3);

    int32_t adc_P = (buffer[0] << 12) | (buffer[1] << 4) | (buffer[2] >> 4);
    uint32_t pressure = bme280_compensate_pressure(adc_P);
    return (float)pressure / 100.0f;
}

// Lê todos os dados (temp, umidade, pressão) de uma vez:
void BME280_ReadAll(I2C_TypeDef *i2c, uint8_t addr, float *temperature, float *humidity, float *pressure) {
    uint8_t buffer[8];

    // Lê 8 bytes a partir do registrador de pressão (0xF7 a 0xFE):
    I2C_ReadRegisters(i2c, addr, BME280_REG_PRESS_MSB, buffer, 8);

    // Extrai os dados brutos:
    int32_t adc_P = (buffer[0] << 12) | (buffer[1] << 4) | (buffer[2] >> 4);
    int32_t adc_T = (buffer[3] << 12) | (buffer[4] << 4) | (buffer[5] >> 4);
    int32_t adc_H = (buffer[6] << 8) | buffer[7];

    // Compensa e converte:
    int32_t temp_comp = bme280_compensate_temperature(adc_T);
    uint32_t press_comp = bme280_compensate_pressure(adc_P);
    int32_t hum_comp = bme280_compensate_humidity(adc_H);

    *temperature = (float)temp_comp / 100.0f;
    *pressure = (float)press_comp / 100.0f;
    *humidity = (float)hum_comp / 1024.0f;
}