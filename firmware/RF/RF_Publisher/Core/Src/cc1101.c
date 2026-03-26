#include "cc1101.h"

// Private Helpers
static void CS_Low(CC1101_t *dev)  { HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_RESET); }
static void CS_High(CC1101_t *dev) { HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_SET); }

void CC1101_WriteReg(CC1101_t *dev, uint8_t addr, uint8_t val) {
    uint8_t tx[2] = {addr, val};
    CS_Low(dev);
    HAL_SPI_Transmit(dev->hspi, tx, 2, 10);
    CS_High(dev);
}

uint8_t CC1101_ReadReg(CC1101_t *dev, uint8_t addr) {
    uint8_t rx = 0;
    addr |= 0x80; // Read bit
    CS_Low(dev);
    HAL_SPI_Transmit(dev->hspi, &addr, 1, 10);
    HAL_SPI_Receive(dev->hspi, &rx, 1, 10);
    CS_High(dev);
    return rx;
}

void CC1101_Strobe(CC1101_t *dev, uint8_t strobe) {
    CS_Low(dev);
    HAL_SPI_Transmit(dev->hspi, &strobe, 1, 10);
    CS_High(dev);
}

uint8_t CC1101_Init(CC1101_t *dev) {
    CS_High(dev);
    HAL_Delay(1);
    CC1101_Strobe(dev, CC1101_SRES);
    HAL_Delay(10);

    if (CC1101_ReadReg(dev, CC1101_VERSION) == 0) return 0; // Fail

    // 915MHz Optimized Config (38.4kBaud, GFSK)
    CC1101_WriteReg(dev, 0x02, 0x06); // IOCFG0: Assert on Sync, de-assert on end
    CC1101_WriteReg(dev, 0x08, 0x05); // PKTCTRL0: Variable length, CRC enabled
    CC1101_WriteReg(dev, 0x06, 0x40); // PKTLEN: 64 bytes max
    CC1101_WriteReg(dev, 0x0D, 0x21); // FREQ2
    CC1101_WriteReg(dev, 0x0E, 0x65); // FREQ1
    CC1101_WriteReg(dev, 0x0F, 0x6A); // FREQ0
    CC1101_WriteReg(dev, 0x10, 0xCA); // MDMCFG4
    CC1101_WriteReg(dev, 0x11, 0x83); // MDMCFG3
    CC1101_WriteReg(dev, 0x12, 0x13); // MDMCFG2: GFSK, 30/32 sync bits
    CC1101_WriteReg(dev, 0x13, 0x22); // MDMCFG1
    CC1101_WriteReg(dev, 0x14, 0xF8); // MDMCFG0
    CC1101_WriteReg(dev, 0x18, 0x18); // MCSM0: Auto-calibrate
    
    CC1101_Strobe(dev, CC1101_SRX); // Default to RX mode
    return 1;
}

void CC1101_SendPacket(CC1101_t *dev, uint8_t *data, uint8_t len) {
    CC1101_Strobe(dev, CC1101_SIDLE);
    CC1101_Strobe(dev, CC1101_SFTX);
    
    CS_Low(dev);
    uint8_t header = CC1101_TXFIFO | 0x40; // Burst Write
    HAL_SPI_Transmit(dev->hspi, &header, 1, 10);
    HAL_SPI_Transmit(dev->hspi, &len, 1, 10); // Length byte
    HAL_SPI_Transmit(dev->hspi, data, len, 10);
    CS_High(dev);
    
    CC1101_Strobe(dev, CC1101_STX);
    
    // Wait for GDO0 to go high (sync sent) then low (packet finished)
    while(HAL_GPIO_ReadPin(dev->gdo0Port, dev->gdo0Pin) == GPIO_PIN_RESET);
    while(HAL_GPIO_ReadPin(dev->gdo0Port, dev->gdo0Pin) == GPIO_PIN_SET);
    
    CC1101_Strobe(dev, CC1101_SRX); // Go back to RX
}

uint8_t CC1101_ReceivePacket(CC1101_t *dev, uint8_t *buf) {
    uint8_t rxBytes = CC1101_ReadReg(dev, CC1101_RXBYTES);
    if (rxBytes < 3) return 0; // Length + 2 status bytes minimum

    CS_Low(dev);
    uint8_t header = CC1101_RXFIFO | 0xC0; // Burst Read
    HAL_SPI_Transmit(dev->hspi, &header, 1, 10);
    
    uint8_t len;
    HAL_SPI_Receive(dev->hspi, &len, 1, 10);
    HAL_SPI_Receive(dev->hspi, buf, len, 10);
    
    uint8_t status[2];
    HAL_SPI_Receive(dev->hspi, status, 2, 10);
    CS_High(dev);

    CC1101_Strobe(dev, CC1101_SFRX);
    CC1101_Strobe(dev, CC1101_SRX);

    return (status[1] & 0x80) ? len : 0; // Return len if CRC is valid
}

void CC1101_SetMaxPower(CC1101_t *dev) {
    // 0x3E is the PATABLE address. 
    // Writing 0xC0 sets the transmit power to +10 dBm (Max).
    CC1101_WriteReg(dev, 0x3E, 0xC0);
}