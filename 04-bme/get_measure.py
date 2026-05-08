import serial
import time
import matplotlib.pyplot as plt
from collections import deque

PORT = 'COM7' 
BAUD = 115200
MAX_POINTS = 50

# Очереди данных
temp_y = deque([0.0] * MAX_POINTS, maxlen=MAX_POINTS)
pres_y = deque([0.0] * MAX_POINTS, maxlen=MAX_POINTS)
hum_y  = deque([0.0] * MAX_POINTS, maxlen=MAX_POINTS)
x_axis = deque(range(MAX_POINTS), maxlen=MAX_POINTS)

def get_value(ser, cmd):
    """Отправляет команду и ждет только строку с данными"""
    ser.write(f"{cmd}\n".encode())
    start_time = time.time()
    while (time.time() - start_time) < 0.2:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if line.startswith(f"{cmd}:"):
            try:
                return float(line.split(':')[1])
            except:
                return None
    return None

plt.ion()
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 8))
fig.canvas.manager.set_window_title('BME280 Telemetry')

def update_plot(ax, data, color, title, unit):
    ax.clear()
    ax.plot(list(x_axis), list(data), color=color, linewidth=2)
    ax.set_title(f"{title}: {data[-1]:.2f} {unit}", fontsize=12, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.set_ylim(min(data)*0.99, max(data)*1.01) 

try:
    ser = serial.Serial(PORT, BAUD, timeout=0.1)
    print(f"--- Подключено к {PORT} ---")
    
    while True:
        t = get_value(ser, "temp")
        p = get_value(ser, "pres")
        h = get_value(ser, "hum")

        if all(v is not None for v in [t, p, h]):
            temp_y.append(t)
            pres_y.append(p)
            hum_y.append(h)

            print(f"\r🌡️ T: {t:5.2f} C | 🌀 P: {p:8.2f} Pa | 💧 H: {h:5.2f} %", end="")

            update_plot(ax1, temp_y, 'red', 'Temperature', '°C')
            update_plot(ax2, pres_y, 'green', 'Pressure', 'Pa')
            update_plot(ax3, hum_y, 'blue', 'Humidity', '%')

            plt.pause(0.05)

except KeyboardInterrupt:
    print("\nОстановка мониторинга...")
finally:
    ser.close()