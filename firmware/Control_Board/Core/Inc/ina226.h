#ifndef INC_INA226_H_
#define INC_INA226_H_

#include "main.h"

#define INA226_ADDR         0x80 // 0x40 << 1

// INA226 Registers
#define INA226_CONFIG_REG   0x00
#define INA226_SHUNT_REG    0x01
#define INA226_BUS_REG      0x02
#define INA226_POWER_REG    0x03
#define INA226_CURRENT_REG  0x04
#define INA226_CALIB_REG    0x05

typedef struct {
    float BusVoltage_V;   // Voltage supplied to the motor
    float Current_mA;     // Current drawn by the motor
    float Power_mW;       // Total power consumption
} INA226_t;

uint8_t INA226_Init(I2C_HandleTypeDef *I2Cx);
void INA226_Read_All(I2C_HandleTypeDef *I2Cx, INA226_t *DataStruct);

#endif /* INC_INA226_H_ */