#ifndef ARDUCAM_H
#define ARDUCAM_H

#include "stm32f4xx_hal.h" // Adjust for your specific Black Pill (F401/F411)
#include <stdint.h>
#include <stdbool.h>

/* Arducam CPLD SPI Registers */
#define ARD_TEST_REG          0x00
#define ARD_CAPTURE_CTRL_REG  0x04
#define ARD_FIFO_CTRL_REG     0x04
#define ARD_STATUS_REG        0x41
#define ARD_FIFO_SIZE1_REG    0x42
#define ARD_FIFO_SIZE2_REG    0x43
#define ARD_FIFO_SIZE3_REG    0x44
#define ARD_FIFO_READ_REG     0x3D

/* CPLD Bit Masks */
#define FIFO_CLEAR_MASK       0x01
#define FIFO_START_MASK       0x02
#define CAP_DONE_MASK         0x08

/* OV2640 I2C / SCCB Address */
#define OV2640_I2C_ADDR       0x60

/* Configuration Structure */
typedef struct {
    I2C_HandleTypeDef *hi2c;
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef      *cs_port;
    uint16_t          cs_pin;
} Arducam_Config_t;

/* Sensor Register Struct for Init Arrays */
struct sensor_reg {
    uint8_t reg;
    uint8_t val;
};

/* Hardware Abstraction Initialization */
void Arducam_Init(Arducam_Config_t *config);

/* SPI CPLD Interface */
void Arducam_WriteReg(uint8_t addr, uint8_t data);
uint8_t Arducam_ReadReg(uint8_t addr);
bool Arducam_TestSPI(void);

/* I2C Sensor Interface */
HAL_StatusTypeDef SCCB_Write(uint8_t regAddr, uint8_t data);
uint8_t SCCB_Read(uint8_t regAddr);

/* Camera Operations */
void Arducam_FlushFIFO(void);
void Arducam_StartCapture(void);
void Arducam_ClearFIFOFlag(void);
bool Arducam_IsCaptureDone(void);
uint32_t Arducam_GetFIFOLength(void);

/* Blocking vs Non-Blocking Reads 
 * Note: For high-bandwidth robotics applications, Arducam_ReadFIFO_DMA 
 * is recommended to prevent blocking the CPU during frame transfer.
 */
void Arducam_ReadFIFO_Blocking(uint8_t *buffer, uint32_t length);
HAL_StatusTypeDef Arducam_ReadFIFO_DMA(uint8_t *buffer, uint32_t length);

/* OV2640 High-Level Config */
void OV2640_InitJPEG(const struct sensor_reg *reg_list);

#endif // ARDUCAM_H