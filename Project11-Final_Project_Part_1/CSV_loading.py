import serial
import csv
import time
from datetime import datetime

# Configure your Serial Port (Update 'COM3' or '/dev/tty...' to match your board)
SERIAL_PORT = 'COM4' 
BAUD_RATE = 9600
CSV_FILE_PATH = 'system_log.csv'

print(f"Connecting to Arduino on {SERIAL_PORT}...")

try:
    # Open serial connection
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2) # Give Arduino 2 seconds to reset/initialize
    print("Connected successfully!")
    
    # Open CSV file and write the header row
    with open(CSV_FILE_PATH, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['PC_Timestamp', 'Arduino_Time_ms', 'Servo_Angle_Deg', 'Fan_Duty', 'Alarm_Active'])
        
        print(f"Logging data to '{CSV_FILE_PATH}'. Press Ctrl+C to stop.\n")
        print("PC Timestamp        | Arduino Ms | Angle | Fan | Alarm")
        print("-" * 55)
        
        while True:
            if ser.in_waiting > 0:
                # Read a line from the serial port
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                # Split the comma-separated values
                data = line.split(',')
                
                # Ensure we got a valid packet (exactly 4 values)
                if len(data) == 4:
                    arduino_time = data[0]
                    angle = data[1]
                    fan_duty = data[2]
                    alarm_state = data[3]
                    
                    # Generate a clean timestamp from your computer's clock
                    pc_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
                    
                    # Write to CSV
                    writer.writerow([pc_time, arduino_time, angle, fan_duty, alarm_state])
                    file.flush() # Instantly write to disk so data isn't lost
                    
                    # Print to terminal so you can watch it live
                    print(f"{pc_time} | {arduino_time:<10} | {angle:<5} | {fan_duty:<3} | {alarm_state}")
                    
except KeyboardInterrupt:
    print("\nLogging stopped by user. File saved!")
except Exception as e:
    print(f"\nAn error occurred: {e}")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()