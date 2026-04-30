#include "mcp9808.h"

/**
 * @brief  Verify sensor and initialize.
 * @return HAL_OK if Manufacturer ID matches 0x0054.
 */
 
HAL_StatusTypeDef MCP9808_Init(I2C_HandleTypeDef *hi2c) {
    uint16_t manufID = MCP9808_ReadID(hi2c);
    if (manufID != 0x0054) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

/**
 * @brief  Reads 16-bit register and handles Big-Endian swap.
 */
static uint16_t MCP9808_ReadReg16(I2C_HandleTypeDef *hi2c, uint8_t reg) {
    uint8_t buf[2];
    if (HAL_I2C_Mem_Read(hi2c, MCP9808_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, buf, 2, 100) != HAL_OK) {
        return 0;
    }
    return (buf[0] << 8) | buf[1];
}

/**
 * @brief  Returns the Manufacturer ID.
 */
uint16_t MCP9808_ReadID(I2C_HandleTypeDef *hi2c) {
    return MCP9808_ReadReg16(hi2c, MCP9808_REG_MANUF_ID);
}

/**
 * @brief  Reads Ambient Temp and converts to Celsius.
 */
float MCP9808_ReadTemperature(I2C_HandleTypeDef *hi2c) {
    uint16_t t_reg = MCP9808_ReadReg16(hi2c, MCP9808_REG_AMBIENT);
    
    // Clear flags (Upper 3 bits)
    float temp = (t_reg & 0x0FFF) / 16.0f;
    
    // Check sign bit (13th bit)
    if (t_reg & 0x1000) {
        temp -= 256.0f;
    }
    
    return temp;
}