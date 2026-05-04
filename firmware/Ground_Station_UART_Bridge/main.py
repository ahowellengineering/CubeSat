import serial
import struct
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# --- CONFIGURATION ---
PORT = '/dev/ttyUSB0' 
BAUD = 115200
SYNC_BYTE = b'\xaa'
plt.style.use('dark_background')
plt.rcParams['figure.dpi'] = 120

ser = serial.Serial(PORT, BAUD, timeout=1)
temp_h, volt_h, opt_h = [], [], []
max_points = 50

# --- WINDOW 1: RF Telemetry ---
fig1, (ax_temp, ax_volt) = plt.subplots(2, 1, figsize=(8, 6))
fig1.canvas.manager.set_window_title('RF Telemetry Dashboard')

# --- WINDOW 2: Optical Link ---
fig2, ax_opt = plt.subplots(1, 1, figsize=(6, 4))
fig2.canvas.manager.set_window_title('Optical Pulse Monitor')

def update(frame):
    # 1. Look for the sync byte
    if ser.read(1) == SYNC_BYTE:
        # 2. Read the 10 bytes of data (4+4+1+1)
        raw_data = ser.read(10) 
        if len(raw_data) == 10:
            # Unpack: float, float, uint8 (battery %), uint8 (optical)
            temp, volt, perc, optical = struct.unpack('<ffBB', raw_data)
            
            temp_h.append(temp)
            volt_h.append(volt)
            opt_h.append(optical)
            
            # Keep history short
            for h in [temp_h, volt_h, opt_h]:
                if len(h) > max_points: h.pop(0)
            
            # --- Update Window 1 ---
            ax_temp.cla()
            ax_temp.plot(temp_h, color='#FF3333', linewidth=1.5)
            ax_temp.set_ylabel("Temp (°C)")
            ax_temp.set_title(f"Thermal: {temp:.1f}°C", loc='right')
            ax_temp.grid(True, alpha=0.1)

            ax_volt.cla()
            ax_volt.plot(volt_h, color='#33FF33', linewidth=1.5)
            ax_volt.set_ylabel("Battery (V)")
            ax_volt.set_title(f"Voltage: {volt:.2f}V ({perc}%)", loc='right')
            ax_volt.grid(True, alpha=0.1)

            # --- Update Window 2 ---
            ax_opt.cla()
            ax_opt.step(range(len(opt_h)), opt_h, color='#3399FF', linewidth=2, where='post')
            ax_opt.set_ylabel("Pulse Count")
            ax_opt.set_title(f"Optical Data: {optical}", color='#3399FF')
            ax_opt.grid(True, alpha=0.1)
            
            # 3. CRITICAL FIX: Manually tell both windows to redraw
            fig1.canvas.draw_idle()
            fig2.canvas.draw_idle()

# Attach animation to fig1, but it updates both in the function
ani = FuncAnimation(fig1, update, interval=100, cache_frame_data=False)

# Keep both windows alive
plt.show()