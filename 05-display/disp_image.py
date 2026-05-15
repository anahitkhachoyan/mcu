import serial
import time
from PIL import Image

# Конфигурация
PORT = 'COM3' 
BAUD = 115200
IMAGE_PATH = 'pics/get.jpg'

def main():
    try:
        connection = serial.Serial(PORT, BAUD, timeout=1)
        
        img = Image.open(IMAGE_PATH).convert('RGB')
        w, h = img.size
        
        print(f"Начинаю загрузку {w}x{h}...")

        for y in range(h):
            for x in range(w):
                r, g, b = img.getpixel((x, y))
                
                # Собираем команду: x y RRGGBB
                command = f"disp_px {x} {y} {r:02X}{g:02X}{b:02X}\n"
                
                connection.write(command.encode())
            
            # Простая индикация прогресса
            if y % 20 == 0:
                print(f"Готово: {int(y/h*100)}%")

        print("Изображение успешно передано!")

    except FileNotFoundError:
        print(f"Ошибка: Не нашел файл по пути {IMAGE_PATH}")
    except serial.SerialException:
        print(f"Ошибка: Не удалось подключиться к {PORT}. Проверь кабель.")
    except Exception as e:
        print(f"Что-то пошло не так: {e}")

    finally:
        time.sleep(0.1)
        if 'connection' in locals() and connection.is_open:
            connection.close()

if __name__ == "__main__":
    main()