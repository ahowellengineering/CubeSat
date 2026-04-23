#include "arducam.h"

// Private instance variable to store HAL handles
static Arducam_Config_t arducam;

void Arducam_Init(Arducam_Config_t *config) {
    arducam = *config;
    
    // Reset the CPLD
    Arducam_WriteReg(0x07, 0x80);
    HAL_Delay(100);
    Arducam_WriteReg(0x07, 0x00);
    HAL_Delay(100);
}

// ---------------------------------------------------------
// SPI CPLD Interface Functions
// ---------------------------------------------------------

void Arducam_WriteReg(uint8_t addr, uint8_t data) {
    uint8_t tx_data[2];
    tx_data[0] = addr | 0x80; // MSB = 1 for CPLD write
    tx_data[1] = data;
    
    HAL_GPIO_WritePin(arducam.cs_port, arducam.cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(arducam.hspi, tx_data, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(arducam.cs_port, arducam.cs_pin, GPIO_PIN_SET);
}

uint8_t Arducam_ReadReg(uint8_t addr) {
    uint8_t tx_data = addr & 0x7F; // MSB = 0 for CPLD read
    uint8_t rx_data = 0;
    
    HAL_GPIO_WritePin(arducam.cs_port, arducam.cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(arducam.hspi, &tx_data, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(arducam.hspi, &rx_data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(arducam.cs_port, arducam.cs_pin, GPIO_PIN_SET);
    
    return rx_data;
}

bool Arducam_TestSPI(void) {
    Arducam_WriteReg(ARD_TEST_REG, 0x55);
    uint8_t temp = Arducam_ReadReg(ARD_TEST_REG);
    return (temp == 0x55);
}

// ---------------------------------------------------------
// I2C / SCCB Sensor Interface Functions
// ---------------------------------------------------------

HAL_StatusTypeDef SCCB_Write(uint8_t regAddr, uint8_t data) {
    return HAL_I2C_Mem_Write(arducam.hi2c, OV2640_I2C_ADDR, regAddr, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

uint8_t SCCB_Read(uint8_t regAddr) {
    uint8_t data = 0;
    HAL_I2C_Mem_Read(arducam.hi2c, OV2640_I2C_ADDR, regAddr, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    return data;
}

// ---------------------------------------------------------
// Capture and FIFO Operations
// ---------------------------------------------------------

void Arducam_FlushFIFO(void) {
    Arducam_WriteReg(ARD_FIFO_CTRL_REG, FIFO_CLEAR_MASK);
}

void Arducam_StartCapture(void) {
    Arducam_WriteReg(ARD_CAPTURE_CTRL_REG, FIFO_START_MASK);
}

void Arducam_ClearFIFOFlag(void) {
    Arducam_WriteReg(ARD_FIFO_CTRL_REG, FIFO_CLEAR_MASK);
}

bool Arducam_IsCaptureDone(void) {
    return (Arducam_ReadReg(ARD_STATUS_REG) & CAP_DONE_MASK);
}

uint32_t Arducam_GetFIFOLength(void) {
    uint32_t len1 = Arducam_ReadReg(ARD_FIFO_SIZE1_REG);
    uint32_t len2 = Arducam_ReadReg(ARD_FIFO_SIZE2_REG);
    uint32_t len3 = Arducam_ReadReg(ARD_FIFO_SIZE3_REG) & 0x7F;
    return ((len3 << 16) | (len2 << 8) | len1);
}

void Arducam_ReadFIFO_Blocking(uint8_t *buffer, uint32_t length) {
    uint8_t cmd = ARD_FIFO_READ_REG;
    
    HAL_GPIO_WritePin(arducam.cs_port, arducam.cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(arducam.hspi, &cmd, 1, HAL_MAX_DELAY);
    
    // Blocking read - acceptable for small frames or simple loops
    HAL_SPI_Receive(arducam.hspi, buffer, length, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(arducam.cs_port, arducam.cs_pin, GPIO_PIN_SET);
}

HAL_StatusTypeDef Arducam_ReadFIFO_DMA(uint8_t *buffer, uint32_t length) {
    uint8_t cmd = ARD_FIFO_READ_REG;
    
    HAL_GPIO_WritePin(arducam.cs_port, arducam.cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(arducam.hspi, &cmd, 1, HAL_MAX_DELAY);
    
    // Triggers DMA stream. Requires CS pin to be pulled HIGH in the HAL_SPI_RxCpltCallback.
    return HAL_SPI_Receive_DMA(arducam.hspi, buffer, length);
}

// ---------------------------------------------------------
// OV2640 High-Level Initialization
// ---------------------------------------------------------

void OV2640_InitJPEG(const struct sensor_reg *reg_list) {
    // Reset the OV2640 sensor
    SCCB_Write(0xFF, 0x01); // Switch to Bank 1
    SCCB_Write(0x12, 0x80); // Software Reset
    HAL_Delay(100);

    // Stream the initialization array over I2C
    for (int i = 0; reg_list[i].reg != 0xFF; i++) {
        SCCB_Write(reg_list[i].reg, reg_list[i].val);
        HAL_Delay(1); 
    }
}