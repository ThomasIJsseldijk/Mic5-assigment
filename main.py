import serial
import time

# Configure the serial port
port = 'COM20'
baud_rate = 115200
timeout = 1  # Timeout for reading from the serial port

# Constants for simulation
MAX_SPEED = 100  # Maximum speed in km/h
MIN_SPEED = 0    # Minimum speed in km/h
MAX_FUEL = 100   # Maximum fuel level in %
MAX_TEMP = 120   # Maximum temperature in °C

lastSpeed = 0    # Initialize lastSpeed to 0
fuel = MAX_FUEL  # Initialize fuel level
last_time = time.time()  # Initialize last_time

def simulate_values(x, y):
    global lastSpeed, fuel, last_time  # Declare as global to modify them

    # Apply dead band to y-axis
    if -4 < y < 4:
        y = 0

    # Calculate speed proportional to forward/backward motion
    if y == 0:  # When y is within the dead band, decrease speed slowly
        lastSpeed *= 0.98  # Slow down gradually
    else:
        lastSpeed += y / 10  # Increase speed with forward motion
    lastSpeed = max(MIN_SPEED, min(MAX_SPEED, lastSpeed))  # Cap speed between MIN_SPEED and MAX_SPEED
    
    # Adjust speed based on left/right turning (x-axis input)
    if x != 0:
        lastSpeed -= abs(x) / 16  # Decrease speed when turning
        lastSpeed = max(MIN_SPEED, min(MAX_SPEED, lastSpeed))  # Cap speed between MIN_SPEED and MAX_SPEED

    # Calculate temperature slowly rising with higher speed
    temperature = 20 + lastSpeed / 2  # Temperature increases with speed

    # Calculate fuel consumption based on speed and y-axis input
    if y != 0:
        fuel_consumption = lastSpeed / 10  # Increase fuel consumption with speed
    else:
        fuel_consumption = 0  # No fuel consumption when coasting

    # Calculate time-based fuel consumption
    current_time = time.time()
    time_elapsed = current_time - last_time
    last_time = current_time

    # Reduce fuel level by speed-based consumption and time-based consumption
    fuel -= fuel_consumption * (time_elapsed * 1)  # Adjust time-based consumption rate as needed
    fuel = max(0, min(MAX_FUEL, fuel))  # Cap fuel between 0 and MAX_FUEL

    return lastSpeed, fuel, temperature

def read_serial():
    # Initialize the serial connection
    with serial.Serial(port, baud_rate, timeout=timeout) as ser:
        while True:
            if ser.in_waiting > 0:
                # Read the incoming data
                line = ser.readline().decode('utf-8').strip()
                if line.startswith('X') and 'Y' in line:
                    try:
                        x_str, y_str = line.split(',')
                        x = int(x_str[1:])
                        y = int(y_str[1:]) * -1  # Invert y-axis
                        print(f"Received X: {x}, Y: {y}")

                        # Simulate values
                        speed, fuel, temperature = simulate_values(x, y)
                        ser.write(f"{int(temperature)},{int(speed)},{int(fuel)}\n".encode('utf-8'))

                    except ValueError:
                        print("Error parsing the values")

if __name__ == "__main__":
    try:
        read_serial()
    except KeyboardInterrupt:
        print("Program terminated")
