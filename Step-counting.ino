#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "[wifi-SSID]";
const char* password = "[wifi-Password]";

// MQTT Broker settings
const char* mqtt_server = "192.168.1.46";
const int mqtt_port = 1883;
const char* mqtt_user = "";      
const char* mqtt_password = "";  

// MQTT Topics (matching Node-RED flow)
const char* topic_steps = "health/steps";
const char* topic_fall = "health/fall";

// Create MPU6050 object
Adafruit_MPU6050 mpu;

// Create OLED display object
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_SDA 19
#define OLED_SCL 18
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET);

// Global Variables for Step Counter
float gyroX[7] = {0, 0, 0, 0, 0, 0, 0};
float filteredGyroX = 0;
int stepCount = 0;
float thresholdUp = 1.1;
float thresholdDown = -1.1;
bool peakDetected = false;
float lastPeak = 0;

// Global Variables for Fall Detection
float fallThreshold = 50.0;  // Threshold for detecting a fall (in m/s²)
bool fallDetected = false;
unsigned long fallTimer = 0;
const unsigned long FALL_DISPLAY_DURATION = 3000;  // Display fall alert for 3 seconds

// MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMqttPublish = 0;
const long mqttPublishInterval = 1000;  // Publish every second

void setupWiFi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
    // Show connection status on OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("Connecting to WiFi"));
    display.display();
  }
  
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Show connected status on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("WiFi Connected!"));
  display.println(WiFi.localIP());
  display.display();
  delay(2000);
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize I2C for MPU6050
  Wire.begin();
  
  // Initialize I2C for OLED
  Wire1.begin(OLED_SDA, OLED_SCL);
  Wire1.setClock(100000);
  
  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1) {
      delay(10);
    }
  }
  
  // Set MPU6050 settings
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 failed"));
    while (1) {
      delay(10);
    }
  }
  
  // Initial display setup
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(15, 5);
  display.println(F("STEP"));
  display.setCursor(15, 25);
  display.println(F("COUNTER"));
  display.display();
  delay(2000);
  
  // Setup WiFi and MQTT
  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);
}

void detectFall(float ax, float ay, float az) {
  // Calculate total acceleration magnitude
  float totalAccel = sqrt(ax*ax + ay*ay + az*az);
  
  // Check if acceleration exceeds fall threshold
  if (totalAccel > fallThreshold && !fallDetected) {
    fallDetected = true;
    fallTimer = millis();
    displayFallAlert();
    
    // Publish fall detection to MQTT
    client.publish(topic_fall, "Fall detected!");
  }
  
  // Reset fall detection after display duration
  if (fallDetected && (millis() - fallTimer >= FALL_DISPLAY_DURATION)) {
    fallDetected = false;
  }
}

void displayFallAlert() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(F("FALL"));
  display.println(F("DETECTED!"));
  display.setTextSize(1);
  display.println(F("\nSeek help if"));
  display.println(F("needed"));
  display.display();
}

void publishToMQTT() {
  if (millis() - lastMqttPublish >= mqttPublishInterval) {
    // Publish step count
    char steps[10];
    itoa(stepCount, steps, 10);
    client.publish(topic_steps, steps);
    
    lastMqttPublish = millis();
  }
}

void loop() {
  // Check MQTT connection
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Print sensor values to Serial Monitor for debugging
  Serial.print("AccelX:"); Serial.print(a.acceleration.x);
  Serial.print(",AccelY:"); Serial.print(a.acceleration.y);
  Serial.print(",AccelZ:"); Serial.print(a.acceleration.z);
  Serial.print(",GyroX:"); Serial.println(g.gyro.x);
  
  // Check for falls
  detectFall(a.acceleration.x, a.acceleration.y, a.acceleration.z);
  
  // Only process steps if no fall is being displayed
  if (!fallDetected) {
    // Update gyro data array for step detection
    for (int j = 0; j < 6; j++) {
      gyroX[j] = gyroX[j + 1];
    }
    gyroX[6] = g.gyro.x;
    
    // Calculate filtered value
    filteredGyroX = (gyroX[0] + gyroX[1] + gyroX[2] + gyroX[3] + 
                     gyroX[4] + gyroX[5] + gyroX[6]) / 7.0;
    
    // Step detection
    if (filteredGyroX > thresholdUp && !peakDetected) {
      peakDetected = true;
      lastPeak = filteredGyroX;
    }
    
    if (peakDetected && filteredGyroX < thresholdDown) {
      stepCount++;
      peakDetected = false;
      updateDisplay();
    }
  }
  
  // Publish data to MQTT
  publishToMQTT();
  
  delay(10);
}

void updateDisplay() {
  display.clearDisplay();
  
  // Display "STEPS:" text
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(F("STEPS:"));
  
  // Display step count in larger text
  display.setTextSize(3);
  display.setCursor(0, 25);
  display.println(stepCount);
  
  display.display();
}
