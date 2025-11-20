#include <WiFi.h>
#include <HTTPClient.h>

// --- 1. CONFIGURATION: REPLACE THESE VALUES ---
const char* ssid = "ZTE Blade V50 Design";      
const char* password = "123802813650";          
const char* serverIp = "10.56.196.107";         
const int serverPort = 80;                      
const char* serverPath = "/esp32_data/post_data.php"; 

// --- 2. SENSOR PIN & CALIBRATION ---
const int voltageSensorPin = 34;    // Sensor OUT connected to GPIO 34 (ADC Pin)

// Typical ESP32 ADC parameters
const int ADC_RESOLUTION = 4095;    // 12-bit ADC
const float ADC_VOLTAGE_REF = 3.3;  // ESP32's internal reference voltage (in Volts)

// CRITICAL: Voltage Divider Ratio (e.g., 5 for a 5:1 divider, allowing up to 16.5V input)
// This value depends entirely on the resistors on your specific sensor module.
const float VOLTAGE_DIVIDER_RATIO = 5.0; 

// --- 3. LOGGING PARAMETERS ---
const long postingInterval = 60000; // Log status every 60 seconds (in ms)
unsigned long lastLogTime = 0;

// --- FUNCTION PROTOTYPES ---
void connectWiFi();
float calculateTrueVoltage();
void sendDataToServer(float voltage);


// --- 4. SETUP FUNCTION ---
void setup() {
  Serial.begin(115200);
  
  // No pinMode needed for ADC input on ESP32
  
  connectWiFi();
  Serial.println("Voltage Sensor Logger Armed.");
}


// --- 5. MAIN LOOP ---
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi(); 
    return; 
  }

  // --- DATA LOGGING ---
  if (millis() - lastLogTime >= postingInterval) {
    lastLogTime = millis();
    
    float trueVoltage = calculateTrueVoltage();
    
    Serial.print("Measured Voltage: ");
    Serial.print(trueVoltage, 2); // Print with 2 decimal places
    Serial.println(" V");
    
    sendDataToServer(trueVoltage); 
  }
  
  // Small delay to prevent crashing the Wi-Fi stack
  delay(10); 
}


// --- 6. CORE LOGIC FUNCTION ---

// Calculates the true voltage applied to the sensor input
float calculateTrueVoltage() {
  // Read the raw analog voltage 10 times for a quick average
  long rawValue = 0;
  for (int i = 0; i < 10; i++) {
    rawValue += analogRead(voltageSensorPin);
    delay(1); 
  }
  float avgRaw = (float)rawValue / 10.0;

  // 1. Convert raw ADC reading to the voltage measured at the ADC pin
  // V_pin = (Raw_ADC / Max_ADC) * V_ref
  float voltageAtPin = (avgRaw / ADC_RESOLUTION) * ADC_VOLTAGE_REF;

  // 2. Apply the voltage divider ratio to find the true input voltage
  // V_true = V_pin * Ratio
  float trueVoltage = voltageAtPin * VOLTAGE_DIVIDER_RATIO;

  return trueVoltage;
}


// --- 7. HELPER FUNCTIONS ---

void connectWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() > 30000 && WiFi.status() != WL_CONNECTED) {
        Serial.println("\nWiFi connection failed, rebooting...");
    }
  }

  Serial.println("\nWiFi connected.");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());
}

void sendDataToServer(float voltage) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverUrl = "http://" + String(serverIp) + ":" + String(serverPort) + String(serverPath);
    
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Create POST data string using the key 'voltage_value'
    char voltageBuffer[10];
    // Format the float to two decimal places for the string
    sprintf(voltageBuffer, "%.2f", voltage);
    
    String postData = "voltage_value=" + String(voltageBuffer);

    Serial.print("Sending POST data: ");
    Serial.println(postData);

    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
      Serial.print("Server Response: ");
      Serial.println(response); 
    } else {
      Serial.print("Error in HTTP request: ");
      Serial.println(httpResponseCode);
    }
    http.end(); 
  } else {
    Serial.println("WiFi Disconnected. Cannot send data.");
  }
}