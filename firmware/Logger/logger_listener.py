import serial
import os

# --- CONFIGURATION ---
# Windows: 'COM3', 'COM4', etc.
# Linux/Mac: '/dev/ttyUSB0', '/dev/tty.usbserial', etc.

# --- CONFIGURATION ---
COM_PORT = '/dev/ttyUSB0'  # Or /dev/ttyACM0

BAUD_RATE = 115200     
OUTPUT_DIR = "captures"
# ---------------------

def listen_for_images():
    # Create a directory to store the images if it doesn't exist
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    try:
        # Open the serial port with a short timeout
        ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=0.1)
        print(f"[*] Listening on {COM_PORT} at {BAUD_RATE} baud...")
        print("[*] Press the RESET button on the Black Pill to capture an image.")
        print("[*] Press Ctrl+C to exit.\n")
        
        buffer = bytearray()
        image_counter = 1
        recording = False
        
        while True:
            # Read whatever is currently in the UART buffer
            chunk = ser.read(1024)
            
            if chunk:
                buffer.extend(chunk)
                
                # 1. Look for the Start of Image (SOI) marker: FF D8
                if not recording:
                    start_idx = buffer.find(b'\xff\xd8')
                    if start_idx != -1:
                        print("[>] Incoming image detected, receiving data...")
                        recording = True
                        # Discard any serial garbage that arrived before the start marker
                        buffer = buffer[start_idx:]
                
                # 2. Look for the End of Image (EOI) marker: FF D9
                if recording:
                    end_idx = buffer.find(b'\xff\xd9')
                    if end_idx != -1:
                        # We found the end marker! Slice the complete image out of the buffer
                        # We add +2 to include the actual FF D9 bytes in the final file
                        image_data = buffer[:end_idx + 2] 
                        
                        filename = os.path.join(OUTPUT_DIR, f"capture_{image_counter}.jpg")
                        with open(filename, "wb") as f:
                            f.write(image_data)
                            
                        print(f"[+] Saved {filename} ({len(image_data)} bytes)")
                        print("[*] Ready for the next image. Press RESET on the board.\n")
                        
                        # Clean up state for the next photo
                        image_counter += 1
                        recording = False
                        
                        # Keep any stray bytes that arrived after the end marker 
                        # (though in your single-shot code, there shouldn't be any)
                        buffer = buffer[end_idx + 2:]

    except serial.SerialException as e:
        print(f"\n[!] Serial Error: {e}")
        print(f"    Make sure {COM_PORT} is correct and not open in another program (like PuTTY or STM32CubeIDE's Serial Monitor).")
    except KeyboardInterrupt:
        print("\n[*] Exiting script...")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == "__main__":
    listen_for_images()