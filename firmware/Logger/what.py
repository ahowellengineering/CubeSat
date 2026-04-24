import serial

try:
    ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=0.5)
    print("[*] Waiting for raw data... Press the STM32 Reset button.")
    
    while True:
        chunk = ser.read(16) # Read 16 bytes at a time
        if chunk:
            print(f"RAW HEX: {chunk.hex(' ').upper()}")
            
except KeyboardInterrupt:
    print("\nExiting.")
finally:
    if 'ser' in locals(): ser.close()