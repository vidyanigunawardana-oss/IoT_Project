#include <WiFi.h>
#include <HTTPClient.h>

// --- 1. CONFIGURATION: REPLACE THESE VALUES ---
const char* ssid = "ZTE Blade V50 Design";      
const char* password = "123802813650";          
const char* serverIp = "10.56.196.107";         
const int serverPort = 80;                     
const char* serverPath = "/esp32_data/post_data.php"; 

// --- 2. SENSOR PIN & CALIBRATION ---
const int currentSensorPin = 34;    // ACS712 OUT connected to GPIO 34 (ADC Pin)

// Calibration values for a 5V/5A ACS712 on 3.3V ESP32
// If you use a 5A sensor: SENSITIVITY is usually 185 mV/A (0.185 V/A)
// If you use a 20A sensor: SENSITIVITY is usually 100 mV/A (0.100 V/A)
// If you use a 30A sensor: SENSITIVITY is usually 66 mV/A (0.066 V/A)
const float ACS_SENSITIVITY = 0.100; // Example: Using 100 mV/A (for a 20A module)

// The offset is the voltage output when 0A current flows. 
// For a 5V sensor powered by 3.3V, the offset is roughly 3.3V / 2 = 1.65V (or 1650 mV)
const int ACS_OFFSET_MV = 1650; 
const int ADC_RESOLUTION = 4095;    // ESP32 ADC is 12-bit
const float ADC_VOLTAGE_REF = 3300.0; // ESP32 Vref is 3300 mV (3.3V)

// --- 3. LOGGING PARAMETERS ---
const long postingInterval = 60000; // Log status every 60 seconds (in ms)
unsigned long lastLogTime = 0;

// --- FUNCTION PROTOTYPES ---
void connectWiFi();
float calculateAmperage();
void sendDataToServer(float amperage);


// --- 4. SETUP FUNCTION ---
void setup() {
  Serial.begin(115200);
  
  connectWiFi();
  Serial.println("Current Sensor Logger Armed.");
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
    
    float currentAmps = calculateAmperage();
    
    Serial.print("Calculated Current: ");
    Serial.print(currentAmps, 2); // Print with 2 decimal places
    Serial.println(" A");
    
    sendDataToServer(currentAmps); 
  }
}


// --- 6. CORE LOGIC FUNCTION ---

// Calculates the true current based on voltage reading
float calculateAmperage() {
  // Read the raw analog voltage 100 times to get a stable average
  long rawValue = 0;
  for (int i = 0; i < 100; i++) {
    rawValue += analogRead(currentSensorPin);
    delay(1); 
  }
  float avgRaw = (float)rawValue / 100.0;

  // 1. Convert raw ADC reading to millivolts (mV)
  float voltageMV = (avgRaw / ADC_RESOLUTION) * ADC_VOLTAGE_REF;

  // 2. Calculate the difference from the offset (0A point)
  float offsetVoltage = voltageMV - ACS_OFFSET_MV;

  // 3. Convert voltage difference to Amps (I = V / SENSITIVITY)
  // Sensitivity is converted to V/A from mV/A for the division
  float current = offsetVoltage / (ACS_SENSITIVITY * 1000.0); 

  // Use the absolute value since we typically only care about magnitude
  return abs(current);
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

void sendDataToServer(float amperage) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverUrl = "http://" + String(serverIp) + ":" + String(serverPort) + String(serverPath);
    
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Create POST data string using the key 'amperage'
    // Format the float to two decimal places for the string
    char currentBuffer[10];
    sprintf(currentBuffer, "%.2f", amperage);
    
    String postData = "amperage=" + String(currentBuffer);

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