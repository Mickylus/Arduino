/*
  UmiditechV2.ino
  Versione per ESP32 Wrover Module (conversione da Arduino UNO R4 WiFi)

  - Access Point + Web server che mostra temperatura, umidita' e volume
  - Sensore DHT11
  - Display I2C LCD (0x27)
  - Buzzer PWM usando LEDC (canale 0)
  - Potenziometro letta tramite ADC (0-4095)

  NOTE sui pin (adattabili):
  - DHT_PIN: GPIO15
  - BUZZ: GPIO27
  - LED: GPIO2 (LED onboard in molte board ESP32)
  - POT: GPIO34 (ADC1)
  - I2C SDA/SCL: default ESP32 (21=SDA, 22=SCL)

  Questa versione evita funzioni specifiche di Arduino UNO e usa API ESP32 standard.
*/

#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>

// Pin configuration (modifica se necessario)
#define DHTPIN 15
#define DHTTYPE DHT11
#define BUZZ 27
#define LED 2
#define POT_PIN 34

// PWM (LEDC) channel for buzzer
const int BUZZ_CHANNEL = 0;
const int BUZZ_FREQ = 2000; // Hz
const int BUZZ_RES = 8;     // 8-bit resolution -> 0-255

// DHT and display
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi AP credentials
const char ssid[] = "UmidiTech";
const char pass[] = "16321632";

WiFiServer server(80);

// State
int flag = 0;
unsigned long millStart = 0;
unsigned long milp = 0;
float h = 0.0, t = 0.0;

void setup() {
  Serial.begin(115200);

  // DHT
  dht.begin();

  // I2C LCD (uses default SDA/SCL pins)
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("UMIDITECH");

  // WiFi AP
  Serial.println("Impostazione Access Point...");
  bool apok = WiFi.softAP(ssid, pass);
  if (!apok) {
    Serial.println("Errore: impossibile avviare AP");
    while (true) { delay(1000); }
  }
  IPAddress ip = WiFi.softAPIP();
  Serial.print("IP ESP32 AP: "); Serial.println(ip);
  server.begin();

  // Pins
  pinMode(LED, OUTPUT);
  pinMode(POT_PIN, INPUT);

  // PWM setup for buzzer
  ledcSetup(BUZZ_CHANNEL, BUZZ_FREQ, BUZZ_RES);
  ledcAttachPin(BUZZ, BUZZ_CHANNEL);

  Serial.println("UmiditechV2 started (ESP32 Wrover)");
}

void loop() {
  unsigned long mill = millis();
  WiFiClient client = server.available();

  // aggiornamento letture ogni 2s
  if (mill - milp >= 2000) {
    milp = mill;
    h = dht.readHumidity();
    t = dht.readTemperature();

    // stampa su seriale e LCD
    Serial.print(h); Serial.print(" "); Serial.print(t); Serial.print(" "); Serial.println(analogRead(POT_PIN));

    lcd.setCursor(0,1);
    lcd.print("U:"); lcd.print(h); lcd.print("% T:"); lcd.print(t); lcd.print((char)223); lcd.print("C");

    // serve client WEB se presente
    stampaServer(client);
  }

  // Gestione buzzer: se attivo, spegni dopo 3.5s
  if (mill - millStart >= 3500) {
    ledcWrite(BUZZ_CHANNEL, 0);
  } else {
    int pot = analogRead(POT_PIN);
    int pwm = map(pot, 0, 4095, 0, 255);
    ledcWrite(BUZZ_CHANNEL, pwm);
  }

  // Allarme umidità
  if (h > 65.0) {
    lcd.setCursor(0,0);
    lcd.print("CAMBIARE ARIA!! ");
    int pot = analogRead(POT_PIN);
    if (flag == 0 && pot > 100) {
      ledcWrite(BUZZ_CHANNEL, 255);
      millStart = millis();
      flag = 1;
    }
    digitalWrite(LED, HIGH);
  } else {
    lcd.setCursor(0,0);
    lcd.print("Regolare        ");
    digitalWrite(LED, LOW);
    ledcWrite(BUZZ_CHANNEL, 0);
    flag = 0;
  }

  delay(10);
}

void stampaServer(WiFiClient client) {
  if (client) {
    String req = client.readStringUntil('\r');
    client.flush();

    float hh = dht.readHumidity();
    float tt = dht.readTemperature();
    int vol = map(analogRead(POT_PIN), 0, 4095, 0, 100);
    vol = constrain(vol, 0, 100);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    client.println("<meta http-equiv='refresh' content='2'>");
    client.println("<style>");
    client.println("body { font-family: Arial; text-align: center; background: #fffff0; }");
    client.println("h1 { color: #1e90ff; font-size: 2.2em; }");
    client.println(".value { font-size: 1.6em; margin: 10px; font-family: Consolas,monaco,monospace; }");
    if (hh > 65.0) {
      client.println(".value2{ font-size: 1.6em; margin: 10px; color: #b22222; font-family: Consolas,monaco,monospace; }");
    } else {
      client.println(".value2{ font-size: 1.6em; margin: 10px; color: #000000; font-family: Consolas,monaco,monospace; }");
    }
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    client.println("<h1>UMIDITECH | DATI</h1>");
    client.print("<p class='value'>Temperatura: "); client.print(tt); client.println(" &deg;C</p>");
    client.print("<p class='value2'>Umidita': "); client.print(hh); client.println(" %</p>");
    client.print("<p class='value'>Volume: "); client.print(vol); client.println("%</p>");
    client.println("</body>");
    client.println("</html>");
    delay(1);
    client.stop();
  }
}
