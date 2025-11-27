/*
  ESP32 - Slave (Sender)
  - Legge joystick X/Y e pulsante, invia ogni 50 ms
  - Invia linea CSV semplice su Serial2: "X,Y,B\n"
  - Baud: 4800, Serial2 TX su default (GPIO17 nella maggior parte delle board)
*/

const int JOY_X_PIN = 34;   // ADC1
const int JOY_Y_PIN = 35;   // ADC1
const int BUTTON_PIN = 32;  // digitale con INPUT_PULLUP

const unsigned long SEND_INTERVAL_MS = 50; // ogni 50 ms
unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  // Serial2: baud 4800, usando i pin hardware di default (RX=16, TX=17 su molte board)
  Serial2.begin(4800);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println("ESP32 sender started (Serial2 4800 baud)");
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

    // Invia come CSV semplice: X,Y,B\n
    Serial2.print(angleX); Serial2.print(',');
    Serial2.print(angleY); Serial2.print(',');

    // Log su USB seriale per debug
    Serial.print("Sent -> ");
    Serial.print(angleX); Serial.print(','); Serial.print(angleY);

    lastSend = now;
  }
  delay(1);
}
