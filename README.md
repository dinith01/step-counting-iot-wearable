
# ESP32 Step Counter and Fall Detection

This project implements a step counter and fall detection system using an ESP32 microcontroller, an MPU6050 sensor, and an SSD1306 OLED display. The system detects steps using gyroscope data, identifies potential falls based on acceleration thresholds, and sends data to an MQTT broker for integration with platforms like Node-RED. The step count and fall alerts are displayed on the OLED screen.

## Features
- **Step Counting**: Uses the MPU6050's gyroscope data to detect and count steps with a moving average filter.
- **Fall Detection**: Detects sudden movements indicative of a fall using accelerometer data and displays an alert on the OLED.
- **WiFi Connectivity**: Connects to a WiFi network to communicate with an MQTT broker.
- **MQTT Integration**: Publishes step count and fall detection events to specified MQTT topics.
- **OLED Display**: Shows real-time step count and fall alerts on a 128x64 SSD1306 OLED display.

## Hardware Requirements
- ESP32 Development Board (e.g., ESP32 DevKitC)
- MPU6050 Sensor (6-axis accelerometer and gyroscope)
- SSD1306 OLED Display (128x64, I2C interface)
- Jumper Wires for connections
- Breadboard (optional, for prototyping)

## Software Requirements
- **Arduino IDE** (version 2.x or later recommended)
- **ESP32 Board Support**: Install via Arduino IDE Boards Manager (search for "ESP32")
- **Libraries**:
  - `Adafruit_MPU6050` (for MPU6050 sensor)
  - `Adafruit_Sensor` (dependency for MPU6050)
  - `Adafruit_GFX` (for OLED graphics)
  - `Adafruit_SSD1306` (for OLED display)
  - `WiFi` (included with ESP32 board support)
  - `PubSubClient` (for MQTT communication)
- **MQTT Broker**: A running MQTT broker (e.g., Mosquitto) on a local network or cloud
- **Node-RED** (optional, for receiving and processing MQTT data)

## Installation

### 1. Set Up Arduino IDE
- Install the Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software).
- Add ESP32 board support by following [these instructions](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html).
- Install the required libraries via the Arduino IDE Library Manager or by downloading from their respective GitHub repositories.

### 2. Connect Hardware
- **MPU6050 to ESP32**:
  - VCC to 3.3V
  - GND to GND
  - SCL to GPIO 22 (default I2C SCL)
  - SDA to GPIO 21 (default I2C SDA)
- **SSD1306 OLED to ESP32**:
  - VCC to 3.3V
  - GND to GND
  - SCL to GPIO 18
  - SDA to GPIO 19
- Ensure proper wiring and verify pin assignments in the code (`OLED_SDA`, `OLED_SCL`).

### 3. Configure the Code
- Open the Arduino sketch (`step_counter_fall_detection.ino`) in the Arduino IDE.
- Update WiFi credentials:
  ```cpp
  const char* ssid = "[wifi-SSID]";
  const char* password = "[wifi-Password]";
  ```
- Update MQTT broker settings:
  ```cpp
  const char* mqtt_server = "192.168.1.46";
  const int mqtt_port = 1883;
  const char* mqtt_user = "";
  const char* mqtt_password = "";
  ```
- Adjust MQTT topics if needed:
  ```cpp
  const char* topic_steps = "health/steps";
  const char* topic_fall = "health/fall";
  ```

### 4. Upload the Code
- Connect the ESP32 to your computer via USB.
- Select the appropriate board and port in the Arduino IDE.
- Upload the sketch to the ESP32.

## Usage
- **Power On**: Connect the ESP32 to a power source (e.g., USB or battery).
- **WiFi Connection**: The ESP32 connects to the specified WiFi network, and the OLED displays the connection status and IP address.
- **Step Counting**: Walk with the device attached to your body (e.g., wrist or waist). The MPU6050 detects steps using gyroscope data, and the step count is displayed on the OLED and published to the `health/steps` MQTT topic.
- **Fall Detection**: If a fall is detected (acceleration exceeds the threshold), the OLED shows a "FALL DETECTED!" alert for 3 seconds, and a message is published to the `health/fall` MQTT topic.
- **Monitoring**: Use Node-RED or another MQTT client to subscribe to the `health/steps` and `health/fall` topics for real-time data monitoring.

## MQTT Integration
- Ensure an MQTT broker is running (e.g., Mosquitto at `192.168.1.46:1883`).
- In Node-RED:
  - Add an MQTT input node for `health/steps` to receive step count updates.
  - Add an MQTT input node for `health/fall` to receive fall detection alerts.
  - Create a flow to process or visualize the data (e.g., display on a dashboard).

## Debugging
- Open the Serial Monitor in the Arduino IDE (115200 baud) to view:
  - WiFi connection status
  - MQTT connection status
  - Raw accelerometer and gyroscope data
  - Step count and fall detection events
- Adjust the step detection thresholds (`thresholdUp`, `thresholdDown`) or fall detection threshold (`fallThreshold`) in the code for better accuracy.

## Notes
- The step detection algorithm uses a moving average filter on gyroscope X-axis data. Calibrate thresholds based on your use case.
- The fall detection threshold (`fallThreshold`) is set to 50 m/s² but may require tuning.
- Ensure the MQTT broker is accessible, and the ESP32 has a stable WiFi connection.
- The OLED display uses a secondary I2C bus (`Wire1`) on pins 18 and 19 to avoid conflicts with the MPU6050.

## Contributing
Contributions are welcome! Please submit a pull request or open an issue on GitHub for bug reports, feature requests, or improvements.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

