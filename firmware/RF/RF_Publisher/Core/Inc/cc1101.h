#ifndef CC1101_H
#define CC1101_H

#include "stm32f4xx_hal.h"

// --- Command Strobes ---
#define CC1101_SRES         0x30      // Reset chip
#define CC1101_SFSTXON      0x31      // Enable freq. synth and allow faster TX
#define CC1101_SXOFF        0x32      // Turn off crystal oscillator
#define CC1101_SCAL         0x33      // Calibrate frequency synthesizer
#define CC1101_SRX          0x34      // Enable RX
#define CC1101_STX          0x35      // Enable TX
#define CC1101_SIDLE        0x36      // Exit RX / TX, turn off freq. synth
#define CC1101_SWOR         0x38      // Wake on Radio
#define CC1101_SPWD         0x39      // Enter power down mode
#define CC1101_SFRX         0x3A      // Flush RX FIFO
#define CC1101_SFTX         0x3B      // Flush TX FIFO
#define CC1101_SNOP         0x3D      // No operation

// --- Status Registers (Burst Bit 0x40 + Read Bit 0x80 = 0xC0) ---
#define CC1101_PARTNUM      (0x30 | 0xC0)
#define CC1101_VERSION      (0x31 | 0xC0)
#define CC1101_FREQEST      (0x32 | 0xC0)
#define CC1101_LQI          (0x33 | 0xC0)
#define CC1101_RSSI         (0x34 | 0xC0)
#define CC1101_MARCSTATE    (0x35 | 0xC0)
#define CC1101_WORTIME1     (0x36 | 0xC0)
#define CC1101_WORTIME0     (0x37 | 0xC0)
#define CC1101_PKTSTATUS    (0x38 | 0xC0)
#define CC1101_VCO_VC_DAC   (0x39 | 0xC0)
#define CC1101_TXBYTES      (0x3A | 0xC0)
#define CC1101_RXBYTES      (0x3B | 0xC0)

// --- Data FIFO ---
#define CC1101_TXFIFO       0x3F
#define CC1101_RXFIFO       0x3F

// --- Device Handle Structure ---
typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *csPort;
    uint16_t csPin;
    GPIO_TypeDef *gdo0Port;
    uint16_t gdo0Pin;
} CC1101_t;

// --- Function Prototypes ---

/**
 * @brief Initializes the CC1101 with 915MHz default settings.
 * @return 1 if success (Version check passed), 0 if failed.
 */
uint8_t CC1101_Init(CC1101_t *dev);

/**
 * @brief Sends a packet of data. Handles SIDLE and SRX transitions.
 */
void CC1101_SendPacket(CC1101_t *dev, uint8_t *data, uint8_t len);

/**
 * @brief Polls for a received packet.
 * @return Length of received data if CRC is valid, 0 otherwise.
 */
uint8_t CC1101_ReceivePacket(CC1101_t *dev, uint8_t *buf);

/**
 * @brief Low-level register access and strobes
 */
void CC1101_WriteReg(CC1101_t *dev, uint8_t addr, uint8_t val);
uint8_t CC1101_ReadReg(CC1101_t *dev, uint8_t addr);
void CC1101_Strobe(CC1101_t *dev, uint8_t strobe);
void CC1101_SetMaxPower(CC1101_t *dev);
int16_t CC1101_GetRSSI(uint8_t buf[], uint8_t buf_len);
uint8_t CC1101_ReadStatus(CC1101_t *dev, uint8_t addr);

#endif