#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>

// --- 1. GLOBAL CONFIGURATION ---
const char* ssid = "ZTE Blade V50 Design";      
const char* password = "123802813650";          
const char* serverIp = "10.180.131.107";         
const char* serverPath = "/esp32_data/post_data.php"; 

const long loggingInterval = 60000; // Log sensor values every 60 seconds (in ms)
unsigned long lastLogTime = 0;

// --- 2. PIN DEFINITIONS (Final Setup) ---
const int dhtPin = 5;           // DHT11 Data -> GPIO 5
const int pirPin = 27;          // PIR Data -> GPIO 27
const int ldrPin = 35;          // LDR Signal -> GPIO 35 (ADC)
const int gasSensorPin = 34;    // Gas Sensor A0 -> GPIO 34 (ADC)
const int relayPin = 4;         // Relay IN -> GPIO 4
const int ledPinPIR = 25;       // LED 1 -> GPIO 25 (PIR indicator)
const int ledPinGAS = 26;       // LED 2 -> GPIO 26 (Gas indicator)

// --- 3. SENSOR CALIBRATION PARAMETERS ---
// GAS SENSOR (MQ Series)
const float RL_VALUE = 10.0;     
const float RO_CLEAN_AIR_VALUE = 9.8; 
const float VCC_VOLTS = 3.3;     
const int ADC_MAX = 4095;        
const int safetyThresholdPPM = 15; // PPM threshold for gas alert

// DHT setup
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE);

// Logic variables
int pirState = LOW; // Tracks motion state

// --- FUNCTION PROTOTYPES ---
void connectWiFi();
void sendData(String table, String data);
float getResistanceRatio(int rawADC);
float getPPM(float RsR0);


// --- 4. SETUP FUNCTION ---
void setup() {
  Serial.begin(115200);
  dht.begin();
  
  pinMode(relayPin, OUTPUT);
  pinMode(ledPinPIR, OUTPUT);
  pinMode(ledPinGAS, OUTPUT);
  
  digitalWrite(relayPin, HIGH); // Relay OFF by default
  digitalWrite(ledPinPIR, LOW);
  digitalWrite(ledPinGAS, LOW);

  pinMode(pirPin, INPUT_PULLUP);
  
  connectWiFi();
  Serial.println("\n--- Integrated System Armed ---");
}


// --- 5. MAIN LOOP ---
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    return;
  }
  
  // --- READ ALL SENSORS ---
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int gasRaw = analogRead(gasSensorPin);
  int ldrVal = analogRead(ldrPin);
  int pirVal = digitalRead(pirPin);
  
  // GAS CALCULATION
  float RsR0 = getResistanceRatio(gasRaw);
  float ppm = getPPM(RsR0); 
  
  // --- A. PIR SENSOR (Immediate Logging) ---
  if (pirVal == HIGH) { 
    if (pirState == LOW) {
      pirState = HIGH;
      digitalWrite(ledPinPIR, HIGH); 
      Serial.println("PIR: Motion Detected!");
      sendData("motion_logs", "motion=DETECTED");
    }
  } else {
    if (pirState == HIGH) {
      pirState = LOW;
      digitalWrite(ledPinPIR, LOW); 
      Serial.println("PIR: Motion Clear.");
      sendData("motion_logs", "motion=CLEAR");
    }
  }
  
  // B. GAS/RELAY CONTROL (Immediate Control)
  if (ppm > safetyThresholdPPM) { 
    digitalWrite(ledPinGAS, LOW); // LED 2 ON
    digitalWrite(relayPin, LOW);   // Relay ON
  } else {
    digitalWrite(ledPinGAS, HIGH);
    digitalWrite(relayPin, HIGH); // Relay OFF
  }

  // --- C. TIMED LOGGING (Every 60 seconds) ---
  if (millis() - lastLogTime >= loggingInterval) {
    lastLogTime = millis();
    
    // 1. DHT11 
    if (!isnan(t) && !isnan(h)) {
        String data = "temperature=" + String(t, 2) + "&humidity=" + String(h, 2);
        sendData("dht_readings", data);
    }

    // 2. LDR 
    String dataLDR = "ldr_value=" + String(ldrVal);
    sendData("ldr_readings", dataLDR);

    // 3. Gas Sensor (PPM Value)
    String dataGas = "ppm_value=" + String(ppm, 1);
    sendData("gas_readings", dataGas);

    // 4. Relay Status (Log current status)
    String relayStatus = (ppm > safetyThresholdPPM) ? "ON" : "OFF";
    sendData("relay_logs", "state=" + relayStatus);
  }
  
  delay(100);
}


// --- 6. CORE LOGIC AND HELPERS (Same as before) ---

// PPM CALCULATION
float getResistanceRatio(int rawADC) {
    if (rawADC == 0) return 0;
    float sensorVoltage = ((float)rawADC / ADC_MAX) * VCC_VOLTS; 
    float RS = ((VCC_VOLTS / sensorVoltage) - 1) * RL_VALUE; 
    return RS / RO_CLEAN_AIR_VALUE; 
}

float getPPM(float RsR0) {
    // Placeholder formula - TUNE THIS
    if (RsR0 > 1.0) return 0.0;
    return pow(10, (log10(RsR0) * -1.8 + 0.3)); 
}

// WIFI CONNECTION (Unchanged)
void connectWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() > 30000) {
        Serial.println("\nWiFi connection failed, please check credentials/hotspot.");
        break; 
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected.");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());
  }
}

// UNIVERSAL DATA SENDER (Unchanged)
void sendData(String table, String data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverUrl = "http://" + String(serverIp) + String(serverPath) + "?table=" + table;
    
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    Serial.print("POST to " + table + ": ");
    Serial.println(data);

    int httpResponseCode = http.POST(data);

    if (httpResponseCode > 0) {
      Serial.print("Code: ");
      Serial.println(httpResponseCode);
      Serial.println("Resp: " + http.getString()); 
    } else {
      Serial.print("Error: ");
      Serial.println(httpResponseCode);
    }
    http.end(); 
  }
}