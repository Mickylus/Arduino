/*
  uno_master_simplified.ino
  Semplice master UNO:
  - SoftwareSerial RX = D8 (ascolta ESP32 TX)
  - Inoltra ogni linea ricevuta via SPI come pacchetto: 0xFF, X, Y, B
  - SPI pins standard: D11 MOSI, D12 MISO, D13 SCK, SS = D10
*/

#include <SoftwareSerial.h>
#include <SPI.h>

SoftwareSerial espSerial(8, 9); // RX = D8, TX = D9 (non usato)
const int SS_PIN = 10;
const int BUF_MAX = 100;
char buf[BUF_MAX];
int bufPos = 0;

void setup() {
  Serial.begin(115200);
  espSerial.begin(4800);
  SPI.begin();
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);
  Serial.println("UNO master simplified started (SoftwareSerial D8, SPI master)");
}

void sendSPIPacket(byte x, byte y, byte b) {
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(0xFF);
  SPI.transfer(x);
  SPI.transfer(y);
  SPI.transfer(b);
  digitalWrite(SS_PIN, HIGH);
}

void processBuffer() {
  if (bufPos == 0) return;
  buf[bufPos] = '\0';
  String s = String(buf);
  s.trim();
  Serial.print("RX raw: "); Serial.println(s);

  // Estrai numeri dalla stringa
  int vals[3] = {0,0,0};
  int vIdx = 0;
  long cur = 0;
  bool inNum = false;

  for (int i = 0; i < s.length() && vIdx < 3; ++i) {
    char c = s.charAt(i);
    if (c >= '0' && c <= '9') {
      inNum = true;
      cur = cur * 10 + (c - '0');
    } else {
      if (inNum) {
        vals[vIdx++] = (int)cur;
        cur = 0; inNum = false;
      }
    }
  }
  if (inNum && vIdx < 3) vals[vIdx++] = (int)cur;

  // Normalizza
  byte x = (vIdx>0) ? constrain(vals[0],0,180) : 0;
  byte y = (vIdx>1) ? constrain(vals[1],0,180) : 0;
  byte b = (vIdx>2) ? (vals[2] != 0 ? 1 : 0) : 0;

  Serial.print("Forwarding SPI: "); Serial.print(x); Serial.print(","); Serial.print(y); Serial.print(","); Serial.println(b);
  sendSPIPacket(x,y,b);
  bufPos = 0;
}

void loop() {
  while (espSerial.available()) {
    int c = espSerial.read();
    if (c == '\n' || c == '\r') {
      if (bufPos > 0) processBuffer();
    } else {
      if (bufPos < BUF_MAX-1) buf[bufPos++] = (char)c;
    }
  }
}
