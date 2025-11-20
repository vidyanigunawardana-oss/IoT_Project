#include <WiFi.h>
#include <HTTPClient.h>

// --- 1. CONFIGURATION: REPLACE THESE VALUES ---
const char* ssid = "ZTE Blade V50 Design";      // Your Wi-Fi network name
const char* password = "123802813650";          // Your Wi-Fi password
const char* serverIp = "10.56.196.107";         // Your XAMPP PC's IPv4 address
const int serverPort = 80;                      // Standard HTTP port
const char* serverPath = "/esp32_data/post_data.php"; 

// --- 2. RELAY PIN DEFINITION ---
const int relayPin = 34;      // Relay IN pin connected to GPIO 16 (Digital Output)

// --- 3. LOGGING PARAMETERS ---
// Change the state every 10 seconds (10000 ms)
const long stateChangeInterval = 10000; 
unsigned long lastStateChangeTime = 0;

// State variables
int relayState = HIGH; // Initialize relay to OFF state (often HIGH is OFF for active-LOW relays)

// --- FUNCTION PROTOTYPES ---
void connectWiFi();
void sendDataToServer(String stateStatus);


// --- 4. SETUP FUNCTION ---
void setup() {
  Serial.begin(115200);
  
  // Initialize the relay pin as an output
  pinMode(relayPin, OUTPUT);
  
  // Set initial state (HIGH for OFF in an active-LOW relay module)
  digitalWrite(relayPin, relayState); 
  
  connectWiFi();
  Serial.println("Relay Control System Armed.");
  
  // Log the initial state
  sendDataToServer("OFF");
}


// --- 5. MAIN LOOP ---
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi(); // Attempt to reconnect if disconnected
    return; 
  }

  // Check if it's time to toggle the relay state
  if (millis() - lastStateChangeTime >= stateChangeInterval) {
    lastStateChangeTime = millis();
    
    // Toggle the state
    if (relayState == HIGH) {
      relayState = LOW; // Turn ON (assuming active-LOW module)
      Serial.println("Relay State: ON");
      sendDataToServer("ON");
    } else {
      relayState = HIGH; // Turn OFF
      Serial.println("Relay State: OFF");
      sendDataToServer("OFF");
    }
    
    // Apply the new state to the physical pin
    digitalWrite(relayPin, relayState);
  }
}


// --- 6. HELPER FUNCTIONS ---

void connectWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
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

void sendDataToServer(String stateStatus) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverUrl = "http://" + String(serverIp) + ":" + String(serverPort) + String(serverPath);
    
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Create POST data string using the key 'state'
    String postData = "state=" + stateStatus;

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