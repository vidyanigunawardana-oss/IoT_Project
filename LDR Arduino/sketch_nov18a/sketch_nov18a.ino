#include <WiFi.h>
#include <HTTPClient.h>

// --- 1. CONFIGURATION: REPLACE THESE VALUES ---
const char* ssid = "ZTE Blade V50 Design";      // Your Wi-Fi network name
const char* password = "123802813650";          // Your Wi-Fi password
const char* serverIp = "10.95.98.107";         // Your XAMPP PC's IPv4 address
const int serverPort = 80;                      // Standard HTTP port
const char* serverPath = "/esp32_data/post_data.php"; 

// --- 2. PIN DEFINITIONS ---
const int ldrPin = 34;      // LDR Analog Signal connected to GPIO 34 (ADC Pin)
const int ledPin = 25;      // LED (via resistor) connected to GPIO 25 (Digital Output)

// --- 3. LOGIC & LOGGING PARAMETERS ---
// ADC on ESP32 is 12-bit by default, giving values from 0 to 4095
const int darkThreshold = 1500; // If LDR value is BELOW this, consider it "dark" (Adjust this value!)
const long postingInterval = 60000; // Log status every 60 seconds (in ms)
unsigned long lastLogTime = 0;

// --- FUNCTION PROTOTYPES ---
void connectWiFi();
void sendDataToServer(int ldrValue);


// --- 4. SETUP FUNCTION ---
void setup() {
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);
  // ADC pin does not require pinMode declaration on ESP32
  
  connectWiFi();
  Serial.println("LDR Logger System Armed.");
}


// --- 5. MAIN LOOP ---
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi(); // Attempt to reconnect if disconnected
    return; 
  }

  // Read the raw analog voltage from the LDR circuit (0 - 4095)
  int ldrValue = analogRead(ldrPin); 

  Serial.print("LDR Raw Value: ");
  Serial.println(ldrValue);

  // --- AUTO-LIGHT LOGIC ---
  if (ldrValue < darkThreshold) {
    // It is dark: turn the LED ON
    digitalWrite(ledPin, HIGH); 
    Serial.println("-> DARK: LED ON");
  } else {
    // It is light: turn the LED OFF
    digitalWrite(ledPin, LOW); 
  }

  // --- DATA LOGGING ---
  if (millis() - lastLogTime >= postingInterval) {
    lastLogTime = millis();
    sendDataToServer(ldrValue); 
  }

  delay(500);
  
}


// --- 6. HELPER FUNCTIONS ---

void connectWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.print(".");
    if (millis() > 30000 && WiFi.status() != WL_CONNECTED) {
        Serial.println("\nWiFi connection failed, rebooting...");
        // ESP.restart(); // Optionally reboot on failure
    }
  }

  Serial.println("\nWiFi connected.");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());
}

void sendDataToServer(int ldrValue) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverUrl = "http://" + String(serverIp) + ":" + String(serverPort) + String(serverPath);
    
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Create POST data string using the key 'ldr_value'
    String postData = "ldr_value=" + String(ldrValue);

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