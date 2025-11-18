#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// --- 1. CONFIGURATION: REPLACE THESE VALUES ---
const char* ssid = "ZTE Blade V50 Design";          // ⬅️ Your Wi-Fi network name
const char* password = "123802813650";  // ⬅️ Your Wi-Fi password
const char* serverIp = "10.95.98.107";       // ⬅️ Your XAMPP PC's IPv4 address
const int serverPort = 80;                    // Standard HTTP port
const char* serverPath = "/esp32_data/post_data.php"; // Path to your PHP script

// DHT Sensor setup
#define DHTPIN 4      // ⬅️ Digital pin connected to the DHT sensor (e.g., GPIO 4)
#define DHTTYPE DHT11 // DHT 11 (AM2302)
DHT dht(DHTPIN, DHTTYPE);

// Interval to send data (in milliseconds: 60000ms = 60 seconds)
const long postingInterval = 60000; 
unsigned long lastUpdateTime = 0;

// --- 2. SETUP FUNCTION ---
void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());
}

// --- 3. MAIN LOOP ---
void loop() {
  if (millis() - lastUpdateTime >= postingInterval) {
    lastUpdateTime = millis();
    sendSensorData();
  }
}

// --- 4. DATA READING AND POSTING FUNCTION ---
void sendSensorData() {
  // Read sensor values
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Temperature as Celsius

  // Check if any reads failed
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Debug output
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" °C, Humidity: ");
  Serial.print(h);
  Serial.println(" %");

  // Prepare HTTP POST request
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverUrl = "http://" + String(serverIp) + ":" + String(serverPort) + String(serverPath);
    
    http.begin(serverUrl);
    // Set the content type expected by the PHP script
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Create the POST data string. Keys MUST match the PHP script's $_POST keys.
    String postData = "temperature=" + String(t) + "&humidity=" + String(h);

    Serial.print("Sending POST data: ");
    Serial.println(postData);

    // Send the request
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
      Serial.print("Server Response: ");
      Serial.println(response); // Shows the success/error message from PHP
    } else {
      Serial.print("Error in HTTP request: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // Free resources
  } else {
    Serial.println("WiFi Disconnected. Cannot send data.");
  }
}