#include "mpu6050.h"

// Scale factors for standard configuration (+-2g, +-250 deg/s)
#define ACCEL_SCALE 16384.0f
#define GYRO_SCALE 131.0f

/**
 * @brief  Initializes the MPU6050 sensor
 * @param  I2Cx: Pointer to I2C handle
 * @retval 0 on success, 1 on failure
 */
uint8_t MPU6050_Init(I2C_HandleTypeDef *I2Cx) {
    uint8_t check;
    uint8_t Data;

    // Check device ID
    HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, 1000);

    if (check == 0x68 || check == 0x98) {  // 0x68 will be returned by the sensor if everything is OK
        // 1. Wake up the sensor (write 0 to PWR_MGMT_1)
        Data = 0;
        HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &Data, 1, 1000);

        // 2. Set Data Rate to 1KHz by writing SMPLRT_DIV register
        Data = 0x07;
        HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, SMPLRT_DIV_REG, 1, &Data, 1, 1000);

        // 3. Set Accelerometer Configuration (+- 2g)
        Data = 0x00;
        HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &Data, 1, 1000);

        // 4. Set Gyroscope Configuration (+- 250 deg/s)
        Data = 0x00;
        HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, GYRO_CONFIG_REG, 1, &Data, 1, 1000);

        return 0; // Success
    }
    
    return 1; // Failure (Device not found)
}

/**
 * @brief  Reads all 14 bytes of data from the sensor
 * @param  I2Cx: Pointer to I2C handle
 * @param  DataStruct: Pointer to the MPU6050 data structure
 */
void MPU6050_Read_All(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct) {
    uint8_t Rec_Data[14];

    // Read 14 bytes starting from ACCEL_XOUT_H
    HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, ACCEL_XOUT_H_REG, 1, Rec_Data, 14, 1000);

    // Combine High and Low bytes
    DataStruct->Accel_X_RAW = (int16_t)(Rec_Data[0] << 8 | Rec_Data[1]);
    DataStruct->Accel_Y_RAW = (int16_t)(Rec_Data[2] << 8 | Rec_Data[3]);
    DataStruct->Accel_Z_RAW = (int16_t)(Rec_Data[4] << 8 | Rec_Data[5]);
    
    DataStruct->Temp_RAW    = (int16_t)(Rec_Data[6] << 8 | Rec_Data[7]);
    
    DataStruct->Gyro_X_RAW  = (int16_t)(Rec_Data[8] << 8 | Rec_Data[9]);
    DataStruct->Gyro_Y_RAW  = (int16_t)(Rec_Data[10] << 8 | Rec_Data[11]);
    DataStruct->Gyro_Z_RAW  = (int16_t)(Rec_Data[12] << 8 | Rec_Data[13]);

    // Convert RAW values to actual physical units
    DataStruct->Ax = DataStruct->Accel_X_RAW / ACCEL_SCALE;
    DataStruct->Ay = DataStruct->Accel_Y_RAW / ACCEL_SCALE;
    DataStruct->Az = DataStruct->Accel_Z_RAW / ACCEL_SCALE;

    DataStruct->Gx = DataStruct->Gyro_X_RAW / GYRO_SCALE;
    DataStruct->Gy = DataStruct->Gyro_Y_RAW / GYRO_SCALE;
    DataStruct->Gz = DataStruct->Gyro_Z_RAW / GYRO_SCALE;

    // Convert Temperature to Celsius (Formula from datasheet)
    DataStruct->Temperature = (DataStruct->Temp_RAW / 340.0f) + 36.53f;
}

#define MPU6050_ADDR 0xD0  // 0x68 shifted left by 1. Change to 0xD2 if AD0 pin is high.

float Read_MPU6050_Gyro_Z(I2C_HandleTypeDef *hi2c) 
{
    uint8_t data[2];
    int16_t raw_gyro_z;
    float rate_z;

    // Read 2 bytes starting from GYRO_ZOUT_H (0x47)
    // Using a 5ms timeout to prevent hanging the RTOS task
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR, 0x47, I2C_MEMADD_SIZE_8BIT, data, 2, 5);

    if (ret == HAL_OK) 
    {
        // Combine the High and Low bytes
        raw_gyro_z = (int16_t)(data[0] << 8 | data[1]);
        
        // Convert raw bits to Degrees Per Second 
        // (Assuming default +/- 250 deg/s scale. Divide by 65.5f if using +/- 500 deg/s)
        rate_z = (float)raw_gyro_z / 131.0f; 
    } 
    else 
    {
        // If I2C fails, return 0 to prevent the PID from violently reacting to garbage data
        rate_z = 0.0f;
    }

    return rate_z;
}