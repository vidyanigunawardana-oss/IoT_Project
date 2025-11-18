#include <WiFi.h>
#include <HTTPClient.h>
// DHT libraries are removed since you are focusing on the PIR sensor now
// #include <Adafruit_Sensor.h> 
// #include <DHT.h> 

// --- 1. CONFIGURATION: CRITICAL VALUES ---
const char* ssid = "ZTE Blade V50 Design";      
const char* password = "123802813650";  
const char* serverIp = "10.95.98.107";  
const char* serverPath = "/esp32_data/post_data.php"; 

const int serverPort = 80;                   

// --- 2. PIR & LED PIN DEFINITIONS ---
const int pirPin = 27;     // PIR Sensor OUTPUT connected to GPIO 27 (Input)
const int ledPin = 25;     // LED (via resistor) connected to GPIO 25 (Output)

// State variables for PIR and Logging
int pirState = LOW;             // Current state of the PIR sensor
int motionDelay = 5000;         // Keep LED on for 5 seconds (5000 ms)
const long postingInterval = 10000; // Log status every 10 seconds (or on motion)
unsigned long lastMotionTime = 0;
unsigned long lastLogTime = 0;

// --- FUNCTION PROTOTYPES ---
void connectWiFi();
void sendDataToServer(String motionStatus);


// --- 3. SETUP FUNCTION ---
void setup() {
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); 

  pinMode(pirPin, INPUT_PULLUP);
  
  connectWiFi();
  Serial.println("PIR Logger System Armed.");
}


// --- 4. MAIN LOOP ---
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi(); // Attempt to reconnect if disconnected
    return; 
  }

  // Read the state of the PIR sensor
  int val = digitalRead(pirPin); 

  if (val == HIGH) { // Motion detected!
    if (pirState == LOW) {
      Serial.println("--- Motion Detected! ---");
      digitalWrite(ledPin, HIGH); // Turn LED ON

      // Log the motion event immediately
      sendDataToServer("DETECTED"); 
      
      pirState = HIGH;
    }
    // Record the time of detection (to handle the delay)
    lastMotionTime = millis();
    
  } else { // No motion detected right now
    
    // Check if the LED ON delay has expired
    if (pirState == HIGH && (millis() - lastMotionTime > motionDelay)) {
      Serial.println("Motion stopped.");
      digitalWrite(ledPin, LOW); // Turn LED OFF
      pirState = LOW;
    }
    
    // Log the current status every 10 seconds (even if no motion)
    if (millis() - lastLogTime >= postingInterval) {
        lastLogTime = millis();
        if (pirState == LOW) {
             sendDataToServer("CLEAR");
        } else {
             // If pirState is HIGH, it means motion just stopped but delay is active, so we log DETECTED again
             sendDataToServer("DETECTED_DELAY");
        }
    }
  }
}

// --- 5. HELPER FUNCTIONS ---

void connectWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    // Emergency exit if connecting takes too long
    if (millis() > 30000 && WiFi.status() != WL_CONNECTED) {
        Serial.println("\nWiFi connection failed, rebooting...");
        ESP.restart(); // Restart the ESP32 to try again
    }
  }

  Serial.println("\nWiFi connected.");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());
}

void sendDataToServer(String motionStatus) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverUrl = "http://" + String(serverIp) + ":" + String(serverPort) + String(serverPath);
    
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // We send a custom key 'motion' with the status
    String postData = "motion=" + motionStatus;

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