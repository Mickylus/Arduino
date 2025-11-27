/*
  esp32_sender_simplified.ino
  Semplice: legge joystick X/Y e pulsante, invia ogni 50 ms
  - Serial2 TX -> Arduino UNO SoftwareSerial RX (es. GPIO17 -> UNO D8)
  - Baud: 4800
*/

const int JOY_X_PIN = 34;   // ADC1
const int JOY_Y_PIN = 35;   // ADC1
const int BUTTON_PIN = 32;  // digitale con INPUT_PULLUP
const int SERIAL2_RX = 16;  // non usato qui
const int SERIAL2_TX = 17;  // TX -> UNO RX (D8)

const unsigned long SEND_INTERVAL_MS = 50; // ogni 50 ms
unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(4800, SERIAL_8N1, SERIAL2_RX, SERIAL2_TX);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  analogSetPinAttenuation(JOY_X_PIN, ADC_11db);
  analogSetPinAttenuation(JOY_Y_PIN, ADC_11db);
  Serial.println("ESP32 simplified sender started (50ms, Serial2 TX=17, 4800 baud)");
}

int mapTo180(int raw) {
  int v = map(raw, 0, 4095, 0, 180);
  return constrain(v, 0, 180);
}

void loop() {
  unsigned long now = millis();
  if (now - lastSend >= SEND_INTERVAL_MS) {
    int rawX = analogRead(JOY_X_PIN);
    int rawY = analogRead(JOY_Y_PIN);
    int angleX = mapTo180(rawX);
    int angleY = mapTo180(rawY);
    int btn = (digitalRead(BUTTON_PIN) == LOW) ? 1 : 0;

    // Format semplice: X:nn,Y:nn,B:n\n
    Serial2.print("X:"); Serial2.print(angleX);
    Serial2.print(",Y:"); Serial2.print(angleY);
    Serial2.print(",B:"); Serial2.println(btn);

    Serial.print("Sent -> ");
    Serial.print("X:"); Serial.print(angleX);
    Serial.print(" Y:"); Serial.print(angleY);
    Serial.print(" B:"); Serial.println(btn);

    lastSend = now;
  }
  delay(1);
}
