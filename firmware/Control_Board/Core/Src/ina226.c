#include "ina226.h"

// Helper function to write 16-bit registers (Handles Big-Endian)
static void INA226_Write16(I2C_HandleTypeDef *I2Cx, uint8_t reg, uint16_t value) {
    uint8_t data[2];
    data[0] = (value >> 8) & 0xFF; // MSB
    data[1] = value & 0xFF;        // LSB
    HAL_I2C_Mem_Write(I2Cx, INA226_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 2, 1000);
}

// Helper function to read 16-bit registers (Handles Big-Endian)
static uint16_t INA226_Read16(I2C_HandleTypeDef *I2Cx, uint8_t reg) {
    uint8_t data[2];
    HAL_I2C_Mem_Read(I2Cx, INA226_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 2, 1000);
    return (uint16_t)((data[0] << 8) | data[1]); // Combine MSB and LSB
}

uint8_t INA226_Init(I2C_HandleTypeDef *I2Cx) {
    // 1. Check if device is on the bus
    if (HAL_I2C_IsDeviceReady(I2Cx, INA226_ADDR, 3, 1000) != HAL_OK) {
        return 1; // Hardware failure (check pull-ups and wiring)
    }

    // 2. Write Configuration: 
    // Defaults to continuous shunt and bus voltage conversions
    // Avg = 1 (No averaging), VBUS conversion time = 1.1ms, Shunt conv = 1.1ms
    INA226_Write16(I2Cx, INA226_CONFIG_REG, 0x4127);

    // 3. Write Calibration Register for 0.1mOhm Shunt and 0.1mA LSB
    INA226_Write16(I2Cx, INA226_CALIB_REG, 0x0200);

    return 0; // Success
}

void INA226_Read_All(I2C_HandleTypeDef *I2Cx, INA226_t *DataStruct) {
    int16_t current_raw;
    uint16_t bus_raw, power_raw;

    // Read raw registers
    bus_raw = INA226_Read16(I2Cx, INA226_BUS_REG);
    current_raw = (int16_t)INA226_Read16(I2Cx, INA226_CURRENT_REG); // Current can be negative!
    power_raw = INA226_Read16(I2Cx, INA226_POWER_REG);

    // Convert to physical units
    // Bus Voltage LSB is always 1.25mV
    DataStruct->BusVoltage_V = (float)bus_raw * 0.00125f; 
    
    // Current LSB was configured to 0.1mA
    DataStruct->Current_mA = (float)current_raw * 0.1f;
    
    // Power LSB is always 25 * Current LSB (25 * 0.1mA = 2.5mW)
    DataStruct->Power_mW = (float)power_raw * 2.5f;
}