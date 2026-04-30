#ifndef MCP9808_H
#define MCP9808_H

#include "stm32f4xx_hal.h" // Change this if using a different series

// Default I2C Address (0x18 << 1)
#define MCP9808_I2C_ADDR      (0x18 << 1)

// Registers
#define MCP9808_REG_CONFIG    0x01
#define MCP9808_REG_UPPER_LIM 0x02
#define MCP9808_REG_LOWER_LIM 0x03
#define MCP9808_REG_CRIT_LIM  0x04
#define MCP9808_REG_AMBIENT   0x05
#define MCP9808_REG_MANUF_ID  0x06
#define MCP9808_REG_DEVICE_ID 0x07
#define MCP9808_REG_RESOLUTION 0x08

// Function Prototypes
HAL_StatusTypeDef MCP9808_Init(I2C_HandleTypeDef *hi2c);
float             MCP9808_ReadTemperature(I2C_HandleTypeDef *hi2c);
uint16_t          MCP9808_ReadID(I2C_HandleTypeDef *hi2c);

#endif