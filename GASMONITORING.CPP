#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid = "SAC-05(2.4GHz)";
const char* password = "sac@1964";

const char* botToken = "7628744401:AAEHenmL5y26NRO9WvPWpGv3JX8UsTh8uXc";
const int chatID = 1956241063;

const int gasSensorPin = A0;
const int ledPin = 5;     // GPIO14
const int buzzerPin = 4;  // GPIO12

const int threshold = 800;  // Tune this based on testing
unsigned long lastAlertTime = 0;
const unsigned long alertInterval = 60000; // 1 min delay between alerts

void setup() {
  Serial.begin(115200);
  pinMode(gasSensorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  connectToWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    reconnectWiFi();
  }

  int gasValue = analogRead(gasSensorPin);
  Serial.println(gasValue);

  if (gasValue > threshold) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
   
    sendTelegramMessage("🚨 Gas Leak Detected! \nSensor Value: "+ String(gasValue));
  delay(15000); // wait 15 seconds to avoid rate limit
    // Send alert if enough time passed since last
   // if (millis() - lastAlertTime > alertInterval) {
   //   sendTelegramMessage("🚨 Gas Leak Detected! \nSensor Value: " + String(gasValue));
   //   lastAlertTime = millis();
    //}
  } else {
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
  }

  delay(1000);
}

void connectToWiFi() {

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.println(WiFi.status());
  }

  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnectWiFi() {
  Serial.println("Reconnecting to WiFi...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nReconnected!");
  } else {
    Serial.println("\nFailed to reconnect.");
  }
}

void sendTelegramMessage(String message) {
  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("Telegram connection failed");
    return;
  }

  String url = "/bot" + String(botToken) + "/sendMessage?chat_id=" + String(chatID) + "&text=" + urlencode(message) + "&parse_mode=Markdown";

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.telegram.org\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("Telegram alert sent");
}

String urlencode(String str) {
  String encoded = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      encoded += '%';
      code0 = (c >> 4) & 0xF;
      code1 = c & 0xF;
      encoded += String("0123456789ABCDEF")[code0];
      encoded += String("0123456789ABCDEF")[code1];
    }
  }
  return encoded;
}