// Blynk details
#define BLYNK_TEMPLATE_ID "TMPL6juufVRGm"
#define BLYNK_TEMPLATE_NAME "Home Automation System"
#define BLYNK_AUTH_TOKEN "zRV7zXzcfNTooLZWABxrj9i0w-OHwEja"

//Libraries
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>

#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

String googleScriptURL = "https://script.google.com/macros/s/AKfycbzQQJvGopWgcDIU531B0txn9QtDomdqfo26SvfYMQqHUanBKk9usY4OIbabWZlsna5q/exec";

//Link to Google sheet
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
      Serial.println(response);  //debugging
    } else {
      Serial.println("Error in sending POST request");
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

// Initialize LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x3F for a 16 chars and 2 line display

// Pin definitions for components
#define trigPin 13
#define echoPin 12

#define Led2 4
#define alarm 2

#define pirLED 23
#define PIR_PIN 35
#define PIR_PIN2 34

#define Door_Lock 5

char key;
int alarmCount;
int virtualPin0Lock;

// LCD and Keypad
#define ROWS 4
#define COLS 4
char keyMap[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

uint8_t rowPins[ROWS] = {14, 27, 26, 25};
uint8_t colPins[COLS] = {33, 32, 18, 19};
Keypad keypad = Keypad(makeKeymap(keyMap), rowPins, colPins, ROWS, COLS);
uint8_t LCD_CursorPosition = 0;
String PassWord = "333333";
String InputStr = "";
int count = 0;
bool locked = false;

// Wi-Fi credentials
char ssid[] = "Ashen";
char password[] = "123456789";

int virtualPin3State = 0;

// Setup function
void setup() {
  
  Serial.begin(115200);

  // Blynk connection
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password, "blynk.cloud", 80);

  // Pin modes
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(Led2, OUTPUT);
  pinMode(alarm, OUTPUT);

  pinMode(PIR_PIN, INPUT);
  pinMode(PIR_PIN2, INPUT);
  pinMode(pirLED, OUTPUT);
  //pinMode(PirLed2, OUTPUT);

  pinMode(Door_Lock, OUTPUT);

  // LCD setup
  lcd.init();
  lcd.clear();
  lcd.backlight();
}

// Measure distance using Ultrasonic sensor
long measureDistance() {
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1;
  return distance;
}

// Function to read the value from virtual pin 3 (from Blynk app)
BLYNK_WRITE(V3) {
  virtualPin3State = param.asInt();  // Read the value from virtual pin 3
  Serial.println("Virtual Pin 4 state: " + String(virtualPin3State));
}

// Main loop
void loop() {
  Blynk.run();

  //PIR
  int motion = digitalRead(PIR_PIN);

  if(motion == HIGH){
    digitalWrite(pirLED, HIGH);
    Blynk.virtualWrite(V4, 1);
    logEvent("Motion Detected");
  }
  else{
    digitalWrite(pirLED, LOW);
    Blynk.virtualWrite(V4, 0);
  }

  // Read the state of the PIR sensor on pin 4
  int motion2 = digitalRead(PIR_PIN2);

  //Required to met these two conditions
  /*if (motion2 == HIGH && virtualPin3State == 1) {
    digitalWrite(PirLed2, HIGH);  // Turn on the LED (Pin 13)
    logEvent("Motion Detected");
    delay(6000);
    digitalWrite(PirLed2, LOW);
    delay(2000);
    Serial.println("Motion detected and Virtual Pin 4 ON, LED ON");
  } 
  else {
    digitalWrite(PirLed2, LOW);   // Turn off the LED
    Serial.println("No motion or Virtual Pin 4 OFF");
  }*/


  // If password is incorrect, trigger alarm and display unauthorized entry
  if (locked) {
    logEvent("Unauthorized Entry");
    lcd.clear();
    Blynk.virtualWrite(V1, 1);
    
    alarmCount = 0;

    while (alarmCount < 5) {  
        lcd.setCursor(0, 0);
        lcd.print("An unauthorized");
        lcd.setCursor(5, 1);
        lcd.print("Entry");
        
        // Ring the alarm
        digitalWrite(alarm, HIGH);  
        delay(500);                 
        digitalWrite(alarm, LOW);   
        delay(500);                 
        alarmCount++;    
    }
    
    locked = false;  
    lcd.clear();
    delay(2000);
    return;
}
  

  // Check distance using the ultrasonic sensor
  long distance = measureDistance();

  if (distance < 8) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Enter Password ");
    InputStr = "";
    LCD_CursorPosition = 0;
    count = 0;

    while (count <= 3) {
      //read Door lock
      virtualPin0Lock = analogRead(Door_Lock);
      key = keypad.getKey();


      if (key) {
        InputStr += key;
        lcd.setCursor(LCD_CursorPosition++, 1);

        if (LCD_CursorPosition == 6) {
          delay(500);
          lcd.clear();
          LCD_CursorPosition = 0;

          if (InputStr == PassWord) {
            lcd.print("Access Granted!");
            digitalWrite(Door_Lock, LOW);
            Blynk.virtualWrite(V0, 1);
            logEvent("Door Opened");
            Serial.println("Unlocking door...");
            delay(10000);
            digitalWrite(Door_Lock, HIGH);
            Blynk.virtualWrite(V0, 0);
            logEvent("Door Closed");
            Blynk.virtualWrite(V2, 0);
            Serial.println("Locking door...");
            count = 0;
            break;
          } else {
            lcd.print("Wrong PassWord!");
            count += 1;
            if (count >= 3) {
              digitalWrite(Door_Lock, HIGH);
              Blynk.virtualWrite(V0, 1);
              delay(500);
              locked = true;
              break;
            }
          }
        } else if (key == 'D') {
          InputStr = "";
          lcd.clear();
          LCD_CursorPosition = 0;
          lcd.print("Enter PassWord");
        } else {
          lcd.print(key);
        }
      }
    }
  } 
  else {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("  Welcome!");
  }

delay(500);
}
