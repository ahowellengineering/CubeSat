import serial
import struct
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# --- CONFIGURATION ---
PORT = '/dev/ttyUSB0' 
BAUD = 115200
SYNC_BYTE = b'\xaa'
plt.style.use('dark_background') # Makes it look like a pro terminal
plt.rcParams['figure.dpi'] = 120 # Higher quality/sharper lines

ser = serial.Serial(PORT, BAUD, timeout=1)
temp_history, volt_history = [], []
max_points = 50

# Setup the Figure with two subplots
fig, (ax_temp, ax_volt) = plt.subplots(2, 1, sharex=True, figsize=(10, 7))
fig.suptitle('Ground Station - Live RF Telemetry', fontsize=16, color='#00FFCC')

def update(frame):
    if ser.read(1) == SYNC_BYTE:
        raw_data = ser.read(9)
        if len(raw_data) == 9:
            temp, volt, perc = struct.unpack('<ffB', raw_data)
            
            temp_history.append(temp)
            volt_history.append(volt)
            
            if len(temp_history) > max_points:
                temp_history.pop(0)
                volt_history.pop(0)
            
            # --- UPDATE TEMPERATURE PLOT ---
            ax_temp.cla()
            ax_temp.plot(temp_history, color='#FF3333', linewidth=2)
            ax_temp.set_ylabel("Temperature (°C)", fontsize=10)
            ax_temp.grid(color='#444444', linestyle='--', linewidth=0.5)
            ax_temp.set_title(f"Current Temp: {temp:.2f} °C", loc='right', color='#FF3333')

            # --- UPDATE VOLTAGE PLOT ---
            ax_volt.cla()
            ax_volt.plot(volt_history, color='#33FF33', linewidth=2)
            ax_volt.set_ylabel("Battery Voltage (V)", fontsize=10)
            ax_volt.set_xlabel("Recent Samples", fontsize=10)
            ax_volt.grid(color='#444444', linestyle='--', linewidth=0.5)
            ax_volt.set_title(f"Battery: {volt:.2f}V ({perc}%)", loc='right', color='#33FF33')
            
            plt.tight_layout(rect=[0, 0.03, 1, 0.95])

ani = FuncAnimation(fig, update, interval=100)
plt.show()