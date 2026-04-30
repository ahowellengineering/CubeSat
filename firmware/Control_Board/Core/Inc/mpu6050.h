#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_

#include "main.h" // Ensures HAL libraries are pulled in

// MPU6050 I2C Address (AD0 connected to GND)
#define MPU6050_ADDR         0xD0 // 0x69 shifted left by 1

// MPU6050 Registers
#define SMPLRT_DIV_REG       0x19
#define ACCEL_CONFIG_REG     0x1C
#define ACCEL_XOUT_H_REG     0x3B
#define TEMP_OUT_H_REG       0x41
#define GYRO_CONFIG_REG      0x1B
#define GYRO_XOUT_H_REG      0x43
#define PWR_MGMT_1_REG       0x6B
#define WHO_AM_I_REG         0x75

// Data Structure to hold sensor readings
typedef struct {
    // Raw integer values
    int16_t Accel_X_RAW;
    int16_t Accel_Y_RAW;
    int16_t Accel_Z_RAW;
    int16_t Gyro_X_RAW;
    int16_t Gyro_Y_RAW;
    int16_t Gyro_Z_RAW;
    int16_t Temp_RAW;

    // Converted floating-point values
    float Ax;
    float Ay;
    float Az;
    float Gx;
    float Gy;
    float Gz;
    float Temperature;
} MPU6050_t;

// Function Prototypes
uint8_t MPU6050_Init(I2C_HandleTypeDef *I2Cx);
void MPU6050_Read_All(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);
float Read_MPU6050_Gyro_Z(I2C_HandleTypeDef *hi2c) ;

#endif /* INC_MPU6050_H_ */