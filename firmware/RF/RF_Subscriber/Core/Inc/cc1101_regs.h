// cc1101_regs.h

#ifndef CC1101_REGS_H
#define CC1101_REGS_H

// -----------------------------------------------
// Configuration Registers
// -----------------------------------------------
#define REG_IOCFG2      0x00
#define REG_IOCFG1      0x01
#define REG_IOCFG0      0x02
#define REG_FIFOTHR     0x03
#define REG_SYNC1       0x04
#define REG_SYNC0       0x05
#define REG_PKTLEN      0x06
#define REG_PKTCTRL1    0x07
#define REG_PKTCTRL0    0x08
#define REG_ADDR        0x09
#define REG_CHANNR      0x0A
#define REG_FSCTRL1     0x0B
#define REG_FSCTRL0     0x0C
#define REG_FREQ2       0x0D
#define REG_FREQ1       0x0E
#define REG_FREQ0       0x0F
#define REG_MDMCFG4     0x10
#define REG_MDMCFG3     0x11
#define REG_MDMCFG2     0x12
#define REG_MDMCFG1     0x13
#define REG_MDMCFG0     0x14
#define REG_DEVIATN     0x15
#define REG_MCSM2       0x16
#define REG_MCSM1       0x17
#define REG_MCSM0       0x18
#define REG_FOCCFG      0x19
#define REG_BSCFG       0x1A
#define REG_AGCCTRL2    0x1B
#define REG_AGCCTRL1    0x1C
#define REG_AGCCTRL0    0x1D
#define REG_WOREVT1     0x1E
#define REG_WOREVT0     0x1F
#define REG_WORCTRL     0x20
#define REG_FREND1      0x21
#define REG_FREND0      0x22
#define REG_FSCAL3      0x23
#define REG_FSCAL2      0x24
#define REG_FSCAL1      0x25
#define REG_FSCAL0      0x26
#define REG_RCCTRL1     0x27
#define REG_RCCTRL0     0x28
#define REG_FSTEST      0x29
#define REG_PTEST       0x2A
#define REG_AGCTEST     0x2B
#define REG_TEST2       0x2C
#define REG_TEST1       0x2D
#define REG_TEST0       0x2E

// -----------------------------------------------
// Strobe Commands
// -----------------------------------------------
#define STROBE_SRES     0x30  // Reset
#define STROBE_SFSTXON  0x31  // Enable TX freq synth
#define STROBE_SXOFF    0x32  // Crystal off
#define STROBE_SCAL     0x33  // Calibrate freq synth
#define STROBE_SRX      0x34  // Enable RX
#define STROBE_STX      0x35  // Enable TX
#define STROBE_SIDLE    0x36  // Idle
#define STROBE_SWOR     0x38  // Wake on radio
#define STROBE_SPWD     0x39  // Power down
#define STROBE_SFRX     0x3A  // Flush RX FIFO
#define STROBE_SFTX     0x3B  // Flush TX FIFO
#define STROBE_SWORRST  0x3C  // Reset WOR timer
#define STROBE_SNOP     0x3D  // No operation

// -----------------------------------------------
// Status Registers (read only, use with 0x80 | 0x40)
// -----------------------------------------------
#define REG_PARTNUM     0xF0
#define REG_VERSION     0xF1
#define REG_FREQEST     0xF2
#define REG_LQI         0xF3
#define REG_RSSI        0xF4
#define REG_MARCSTATE   0xF5
#define REG_WORTIME1    0xF6
#define REG_WORTIME0    0xF7
#define REG_PKTSTATUS   0xF8
#define REG_VCO_VC_DAC  0xF9
#define REG_TXBYTES     0xFA
#define REG_RXBYTES     0xFB

// -----------------------------------------------
// FIFO
// -----------------------------------------------
#define REG_TXFIFO      0x3F
#define REG_RXFIFO      0x3F  // differentiated by R/W bit

// -----------------------------------------------
// SPI Access Flags
// -----------------------------------------------
#define CC1101_READ     0x80
#define CC1101_BURST    0x40

// -----------------------------------------------
// MARCSTATE values (useful for state machine debug)
// -----------------------------------------------
#define MARCSTATE_IDLE          0x01
#define MARCSTATE_RX            0x0D
#define MARCSTATE_TX            0x13
#define MARCSTATE_RXFIFO_OVF    0x11
#define MARCSTATE_TXFIFO_UNF    0x16

#endif // CC1101_REGS_H