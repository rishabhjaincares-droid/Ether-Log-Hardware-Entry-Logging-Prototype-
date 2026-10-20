#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>        // NEW: Wi-Fi Library
#include <HTTPClient.h>  // NEW: HTTP Web Request Library

// --- WI-FI & SERVER SETTINGS (Loaded from secrets.h) ---
#include "secrets.h" 

// --- PIN DEFINITIONS ---
#define SS_PIN 5
#define RST_PIN 27
#define DOOR_SENSOR_PIN 4
#define GREEN_LED 2
#define RED_LED 15
#define BUZZER_PIN 13

// --- OLED SETUP ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- AUTHORIZED CARDS ---
String authorizedCards[] = {
  "73 D6 FD 28"     
};
int numberOfAuthorizedCards = sizeof(authorizedCards) / sizeof(authorizedCards[0]);

MFRC522 rfid(SS_PIN, RST_PIN);

// --- SYSTEM STATE VARIABLES ---
int lastDoorState = LOW; 
String lastValidUser = "UNKNOWN"; 
unsigned long unlockTimer = 0;    

void setup() {
  Serial.begin(115200);
  
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP); 
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }
  
  // Connect to Wi-Fi
  updateDisplay("CONNECTING...", "Wi-Fi Network");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
  
  SPI.begin();
  rfid.PCD_Init();
  
  updateDisplay("SYSTEM READY", "Scan a card");
}

void loop() {
  // 1. CHECK FOR DOOR TAMPERING
  int currentDoorState = digitalRead(DOOR_SENSOR_PIN);
  if (currentDoorState != lastDoorState) {
    if (currentDoorState == HIGH) { 
      
      if (millis() < unlockTimer) {
        updateDisplay("DOOR OPENED", lastValidUser);
        sendToBlockchain("DOOR_OPENED", lastValidUser);
      } else {
        updateDisplay("ALARM!", "FORCED ENTRY");
        sendToBlockchain("FORCED_ENTRY", "UNKNOWN_INTRUDER");
      }
      
      delay(2000); 
      updateDisplay("SYSTEM READY", "Scan a card");
    }
    lastDoorState = currentDoorState;
    delay(50); 
  }

  // 2. CHECK FOR RFID CARDS
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return; 
  }

  String scannedUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    scannedUID += String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    scannedUID += String(rfid.uid.uidByte[i], HEX);
  }
  scannedUID.trim();
  scannedUID.toUpperCase();
  
  // 3. AUTHORIZATION LOGIC
  bool isAuthorized = false; 
  for (int i = 0; i < numberOfAuthorizedCards; i++) {
    if (scannedUID == authorizedCards[i]) {
      isAuthorized = true; 
      break;               
    }
  }

  // 4. GRANT OR DENY ACCESS
  if (isAuthorized == true) {
    updateDisplay("ACCESS", "GRANTED");
    lastValidUser = scannedUID; 
    
    grantAccess(); 
    unlockTimer = millis() + 10000; 
    
    sendToBlockchain("AUTHORIZED_ENTRY", scannedUID);
  } else {
    updateDisplay("ACCESS", "DENIED!");
    denyAccess();
    sendToBlockchain("ACCESS_DENIED", scannedUID);
  }

  updateDisplay("SYSTEM READY", "Scan a card");
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// --- HELPER FUNCTIONS ---
void grantAccess() {
  digitalWrite(GREEN_LED, HIGH);
  tone(BUZZER_PIN, 2700, 200); 
  delay(3000);                 
  digitalWrite(GREEN_LED, LOW);
}

void denyAccess() {
  digitalWrite(RED_LED, HIGH);
  
  // Rapid 3-beep alarm using the loudest frequency (2700 Hz)
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 2700, 150); // Loud, short beep
    delay(200);                  // Short pause
  }
  
  delay(400); // Wait for the remaining time
  digitalWrite(RED_LED, LOW);
}

// ---> NEW: SEND DATA OVER WI-FI <---
void sendToBlockchain(String eventType, String cardID) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName); 
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"event\":\"" + eventType + "\",\"uid\":\"" + cardID + "\"}";
    
    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end(); // Free resources
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void updateDisplay(String line1, String line2) {
  display.clearDisplay();
  display.setTextSize(2);      
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println(line1);
  display.setTextSize(1);      
  display.setCursor(0, 40);
  display.println(line2);
  display.display();           
}
