#define BLYNK_TEMPLATE_ID "TMPL630ybSgKf"
#define BLYNK_TEMPLATE_NAME "System2"
#define BLYNK_AUTH_TOKEN "D-3e9miz507JRBS7hmuk4qYpFfaSh0lc"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <DHT.h>
#include <ESPAsyncWebServer.h>  // Include for the web server

// Google Sheets Script URL
String googleScriptURL = "https://script.google.com/macros/s/AKfycbzQQJvGopWgcDIU531B0txn9QtDomdqfo26SvfYMQqHUanBKk9usY4OIbabWZlsna5q/exec";

// Function to log events to Google Sheets
void logEvent(String event) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(googleScriptURL);
    http.addHeader("Content-Type", "application/json");

    // JSON payload
    StaticJsonDocument<200> doc;
    doc["time"] = String(hour()) + ":" + String(minute()) + ":" + String(second());
    doc["event"] = event;

    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Google Sheets Response: " + response);
    } else {
      Serial.println("Error in sending POST request to Google Sheets");
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected, can't log event");
  }
}

// Define pins
#define LedWall 2        
#define PIR_PIN 5        
#define PIR_LED 13       
#define MQ_PIN 34       
#define LDR_PIN 35       
#define DHT_PIN 4      
#define BUZZER_PIN 12   

// Initialize DHT sensor
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// Threshold levels
#define GAS_THRESHOLD 450   
#define LIGHT_THRESHOLD 500 
#define TEMP_THRESHOLD 39   
#define HUMID_THRESHOLD 30  

char ssid[] = "Ashen"; 
char password[] = "123456789";  

// Set up the web server on port 80
AsyncWebServer server(80);

float temperature = 0.0;
float humidity = 0.0;
int gasValue = 0;
int lightValue = 0;

void setup() {
  // Initialize serial monitor
  Serial.begin(115200);

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password, "blynk.cloud", 80);

  // Initialize sensors
  dht.begin();

  pinMode(LedWall, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(PIR_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Set up the web server routes
  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("state")) {
      String state = request->getParam("state")->value();
      if (state == "on") {
        digitalWrite(LedWall, HIGH);
        request->send(200, "text/plain", "Wall Lights are ON");
        logEvent("Wall Lights are ON");
      } else if (state == "off") {
        digitalWrite(LedWall, LOW);
        request->send(200, "text/plain", "Wall Lights are OFF");
        logEvent("Wall Lights are OFF");
      } else {
        request->send(400, "text/plain", "Invalid state. Use 'on' or 'off'.");
      }
    } else {
      request->send(400, "text/plain", "Missing 'state' parameter.");
    }
  });

  // Real-time sensor data endpoint
  server.on("/sensor_data", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<200> doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["gasValue"] = gasValue;
    doc["lightValue"] = lightValue;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Start the server
  server.begin();

  Serial.println("System Initialized!");
}

void loop() {
  Blynk.run();


  int motion = digitalRead(PIR_PIN);
  if (motion == HIGH) {
    digitalWrite(PIR_LED, HIGH);
    Serial.println("Motion detected!");
  } else {
    digitalWrite(PIR_LED, LOW);
  }

  gasValue = analogRead(MQ_PIN);
  lightValue = analogRead(LDR_PIN);
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  Blynk.virtualWrite(V0, gasValue);
  Blynk.virtualWrite(V1, temperature);

  Serial.print("Gas Value: ");
  Serial.println(gasValue);
  Serial.print("Light Value: ");
  Serial.println(lightValue);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.println(WiFi.localIP().toString());
  Serial.println("--------------------------");

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000); 
    return;
  }

  // Check thresholds
  if (gasValue > GAS_THRESHOLD &&
      lightValue > LIGHT_THRESHOLD &&
      temperature > TEMP_THRESHOLD &&
      humidity > HUMID_THRESHOLD) {
    // Trigger alarm
    Serial.println("Alarm triggered!");
    logEvent("Fire Alarm Triggered");
    Blynk.virtualWrite(V2, 1);

    int count = 0;

    while (count < 5) { 
      digitalWrite(BUZZER_PIN, HIGH);  
      delay(1000);
      digitalWrite(BUZZER_PIN, LOW);
      delay(500);                      

      digitalWrite(BUZZER_PIN, HIGH);  
      delay(300);
      digitalWrite(BUZZER_PIN, LOW);
      delay(300);

      digitalWrite(BUZZER_PIN, HIGH);  
      delay(300);
      digitalWrite(BUZZER_PIN, LOW);
      delay(1000);                    

      count++;
    }

  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  delay(5000);
}
