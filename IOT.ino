#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspUdp.h>

// Initialize the LCD with I2C address, 16 columns, and 2 rows
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// Wi-Fi credentials
const char* ssid = "Redmi 10";         // Replace with your Wi-Fi SSID
const char* password = "Password";    // Replace with your Wi-Fi password

// ThingSpeak API details
const char* server = "api.thingspeak.com";
const char* apiKey = "FNAC3BU1B0J95367s";    // Replace with your ThingSpeak Write API Key

WiFiEspClient client;  // Initialize Wi-Fi client

// Sensor pins
const int motionSensorPin1 = A0;
const int motionSensorPin2 = A1;

void setup() {
  Serial.begin(9600);

  // Initialize LCD
  lcd.init();              // Initialize the LCD
  lcd.backlight();         // Turn on the backlight
  lcd.setCursor(0, 0);
  lcd.print("SIGN LANGUAGE");

  pinMode(13, OUTPUT);     // Set pin 13 as output for an indicator LED

  // Initialize Wi-Fi
  Serial.println("Initializing Wi-Fi...");
  WiFi.init(&Serial);      // Initialize the WiFi library
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present. Stopping.");
    while (true);          // Stop execution if no Wi-Fi module
  }

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Read sensor values
  int s1 = analogRead(motionSensorPin1);
  int s2 = analogRead(motionSensorPin2);

  Serial.print("Sensor 1: ");
  Serial.println(s1);
  Serial.print("Sensor 2: ");
  Serial.println(s2);

  String message = "NOTHING";

  if (s1 < 150) {
    digitalWrite(13, LOW);
    message = "WATER";
    Serial.println("Detected: WATER");
    lcd.setCursor(0, 1);
    lcd.print("WATER   ");  // Spaces to clear previous text
  } 
  else if (s2 < 200) {
    digitalWrite(13, HIGH);
    message = "MEDICINE";
    Serial.println("Detected: MEDICINE");
    lcd.setCursor(0, 1);
    lcd.print("MEDICINE");
  } 
  else {
    Serial.println("Detected: NOTHING");
    lcd.setCursor(0, 1);
    lcd.print("NOTHING ");  // Spaces to clear previous text
  }

  // Send data to ThingSpeak
  sendToThingSpeak(message);
  
  // Wait before the next iteration
  delay(15000); // ThingSpeak accepts data every 15 seconds
}

void sendToThingSpeak(String message) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWi-Fi reconnected.");
  }

  if (client.connect(server, 80)) {
    Serial.println("Connected to ThingSpeak.");
    
    String postData = "api_key=" + String(apiKey) + "&field1=" + message;

    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.print(postData);

    Serial.println("Data sent to ThingSpeak:");
    Serial.println(postData);

    // Wait for the response from the server
    while (client.connected()) {
      if (client.available()) {
        String response = client.readString();
        Serial.println("Response from ThingSpeak:");
        Serial.println(response);
        break;
      }
    }
  } else {
    Serial.println("Failed to connect to ThingSpeak.");
  }

  client.stop(); // Close the connection
}
