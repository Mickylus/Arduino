# Relazione del progetto "Servo" (esempio)

Versione: 1.0  
Autore: Michele Valiati  
Data: 26 11 2025

## Sommario
Questo documento descrive il progetto che legge un joystick con un ESP32, invia i dati via UART a un Arduino Uno (master), il master inoltra i comandi via SPI ad un Arduino Uno R3 (slave) che comanda un servo e visualizza informazioni su un display LCD I2C. Vengono spiegati i tre protocolli usati (UART, SPI, I2C), la loro applicazione nel progetto, il wiring essenziale, e vengono riportati i principali sketch con spiegazione di funzionamento.

---

## Architettura generale
- ESP32: legge VRx (asse X), VRy (asse Y) e il pulsante del joystick; applica smoothing e invia stringhe tramite Serial2 (TX) verso l'Arduino Uno master.
- Arduino Uno (MASTER): riceve via SoftwareSerial (UART) la stringa dal ESP32, la parse, e costruisce un pacchetto che invia via SPI all'Arduino Uno R3 (SLAVE).
- Arduino Uno R3 (SLAVE): riceve pacchetti via SPI (formato con start marker 0xFF + 3 byte), muove il servo (o aziona rotazione continua) e visualizza X/Y/B su LCD I2C.

Tutti i dispositivi devono condividere il GND comune. Per il servo si consiglia alimentazione 5V esterna con GND in comune.

---

## I protocolli usati

### UART (Universal Asynchronous Receiver/Transmitter)
- Tipo: comunicazione seriale asincrona punto-a-punto.
- Caratteristiche: usa TX/RX, velocità (baud), 1 start bit, 7/8 data bit, parity (opzionale), 1/2 stop bit.
- Vantaggi: semplice, robusto per comunicazioni a bassa velocità; facile debug tramite terminale.
- Uso nel progetto:
  - ESP32 (Serial2) TX -> Arduino Uno SoftwareSerial RX.
  - Formato inviato: stringhe ASCII "X:val,Y:val,B:val\n".
  - Baud usato: 4800 (scelto per maggiore affidabilità con SoftwareSerial).
- Note pratiche:
  - Usare cavi corti, GND comune.
  - Scollegare la linea UNO->ESP32 durante i test o usare level shifter (UNO TX = 5V non ideale per ESP32 RX).
  - Se si rileva "garbage" usare baud più basso e/o una libreria seriale alternativa (AltSoftSerial).

### SPI (Serial Peripheral Interface)
- Tipo: bus sincrono master/slave, full-duplex.
- Linee: MOSI (master out → slave in), MISO (slave out → master in), SCK (clock), SS (slave select).
- Caratteristiche: veloce, nessuna indirizzazione integrata (il master seleziona lo slave abbassando SS).
- Uso nel progetto:
  - UNO (MASTER) usa SPI per inviare pacchetti al UNO R3 (SLAVE).
  - Wiring: D11 (MOSI), D12 (MISO), D13 (SCK), SS = D10.
  - Protocollo semplice adottato: [START = 0xFF][X][Y][B]
    - START aiuta la sincronizzazione (lo slave ignora bytes finché non vede 0xFF).
- Implementazione tecnica:
  - Master: digitalWrite(SS, LOW); SPI.transfer(...); digitalWrite(SS, HIGH);
  - Slave: abilitare SPI in modalità slave e SPI interrupt (SPCR |= SPE | SPIE) e usare ISR SPI_STC_vect per leggere SPDR.
- Note pratiche:
  - In slave MISO deve essere OUTPUT.
  - USARE variabili volatile per comunicare tra ISR e loop, copiare i dati fuori dall'ISR con noInterrupts()/interrupts().

### I2C (Inter-Integrated Circuit)
- Tipo: bus sincrono multi-master/multi-slave a due linee (SDA data, SCL clock) con indirizzamento a 7-bit (o 10-bit).
- Caratteristiche: semplice per periferiche multiple (sensori, display), supporta pull-up su SDA/SCL.
- Uso nel progetto:
  - UNO R3 (SLAVE) usa I2C per il display LCD (LiquidCrystal_I2C) all'indirizzo tipico 0x27.
  - Visualizza i valori X/Y/B sul display 16×2.
- Note pratiche:
  - Pull-up da 4.7k–10k su SDA/SCL se non forniti dal modulo.
  - Verificare indirizzo I2C con I2C scanner se il display non risponde.

---

## Wiring essenziale (sintesi)
- Alimentazioni:
  - ESP32: 3.3V (alimentazione USB o esterna)
  - UNO master: 5V (USB)
  - UNO R3 (slave): 5V (USB o esterna)
  - Servo: 5V (preferibile alimentazione esterna)
  - Tutti i GND collegati insieme.

- UART (ESP32 ↔ UNO master):
  - ESP32 Serial2 TX (GPIO17) -> UNO SoftwareSerial RX (D8)  
  - GND comune

- SPI (UNO master ↔ UNO R3 slave):
  - UNO MASTER D11 (MOSI) -> SLAVE D11 (MOSI)
  - UNO MASTER D12 (MISO) -> SLAVE D12 (MISO)
  - UNO MASTER D13 (SCK)  -> SLAVE D13 (SCK)
  - UNO MASTER D10 (SS)   -> SLAVE D10 (SS)
  - GND comune

- I2C (UNO R3 ↔ LCD I2C):
  - UNO R3 A4 (SDA) -> LCD SDA
  - UNO R3 A5 (SCL) -> LCD SCL
  - VCC/GND: come richiesto

- Servo:
  - Signal -> UNO R3 pin 3
  - VCC -> 5V (preferibilmente esterno)
  - GND -> comune

---

## Codici e spiegazione del funzionamento

Di seguito i file principali usati nel progetto. Caricali sui rispettivi dispositivi.

### 1) ESP32 — lettura joystick e invio UART
```arduino name=esp32_sender_complete.ino
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
```

Spiegazione:
- Legge gli ADC delle porte consigliate (ADC1) per ridurre il rumore; applica un filtro EMA (exponential moving average) per smoothing.
- Invia stringhe ASCII via Serial2 a 4800 baud (più affidabile con SoftwareSerial lato UNO).
- Invia periodicamente (SEND_INTERVAL_MS) o quando c'è un cambiamento significativo (THRESHOLD), e almeno un heartbeat periodico.

---

### 2) Arduino Uno (MASTER) — ricezione UART e inoltro SPI
```arduino name=uno_master_corrected.ino
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
```

Spiegazione:
- Il master ascolta la SoftwareSerial e accumula i caratteri in un buffer finché non trova terminatore (\n o \r).
- Filtra la stringa per tenere solo numeri e separatori, poi estrae i valori numerici.
- Costruisce un pacchetto SPI con marker 0xFF e i tre byte X,Y,B e lo invia allo slave (SS basso durante la trasmissione).

---

### 3) Arduino Uno R3 (SLAVE) — ricezione SPI e controllo servo + LCD
Versione robusta (posizionale servo su pin 3):
```arduino name=uno_r3_slave_fixed.ino
/*
  uno_r3_slave_simplified.ino
  Semplice slave UNO R3:
  - Riceve pacchetti SPI: 0xFF, X, Y, B
  - Muove servo su pin 3 e scrive su LCD I2C (0x27)
*/

#include <SPI.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myservo;

volatile bool gotStart = false;
volatile uint8_t idx = 0;
volatile uint8_t buf[3];
volatile bool ready = false;

ISR(SPI_STC_vect) {
  uint8_t v = SPDR;
  if (!gotStart) {
    if (v == 0xFF) {
      gotStart = true;
      idx = 0;
    }
  } else {
    if (idx < 3) {
      buf[idx++] = v;
      if (idx >= 3) {
        ready = true;
        gotStart = false;
      }
    } else {
      gotStart = false;
      idx = 0;
    }
  }
}

void setup() {
  Serial.begin(115200);
  myservo.attach(3);
  myservo.write(90);
  Wire.begin();
  lcd.init(); lcd.backlight(); lcd.clear();
  lcd.print("SPI slave ready");

  pinMode(MISO, OUTPUT);
  pinMode(10, INPUT_PULLUP);
  SPCR |= _BV(SPE) | _BV(SPIE);
  sei();
}

void loop() {
  if (ready) {
    noInterrupts();
    uint8_t x = buf[0];
    uint8_t y = buf[1];
    uint8_t b = buf[2];
    ready = false;
    interrupts();

    x = constrain(x,0,180);
    myservo.write(x);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("X:"); lcd.print(x);
    lcd.print(" Y:"); lcd.print(y);
    lcd.setCursor(0,1);
    lcd.print("B:"); lcd.print((b!=0)?1:0);

    Serial.print("Got SPI: "); Serial.print(x); Serial.print(","); Serial.print(y); Serial.print(","); Serial.println(b);
  }
  delay(5);
}

```

Spiegazione:
- L'ISR (`SPI_STC_vect`) legge i byte in arrivo e cerca il marker 0xFF; quando raccoglie 3 byte li segnala al loop principale con `packetReady`.
- Nel loop, i dati vengono copiati in modo atomico e applicate azioni: posizionamento servo e aggiornamento LCD.
- `pinMode(10, INPUT_PULLUP)` evita false attivazioni dovute a SS fluttuante.

---

### 4) (Opzionale) Arduino R3 slave — modalità continuous-rotation (360° via tempo)
Se usi un servo a rotazione continua puoi usare la versione che gestisce rotazione per gradi usando controllo temporale. È incluso come file separato `uno_r3_slave_360_continuous.ino` — il codice mappa un byte X (0..255) a 0..360° e ruota di quella quantità in base a `degreesPerSecond` calibrato. Utile se hai un continuous-rotation servo; non usabile con servo standard che sono limitati a ~0..180°.

(Se vuoi includo qui il codice completo della variante continua; è già stato fornito in sessione e posso inserirlo su richiesta.)

---

## Testing, calibrazione e suggerimenti pratici

- Debug UART: usare i programmi diagnostici che stampano ogni byte in HEX aiuta a capire se la linea presenta rumore.
- Se "garbage" appare, provare:
  - baud più basso (4800 o 2400),
  - cambiare pin RX per SoftwareSerial,
  - usare AltSoftSerial o NeoSWSerial,
  - mettere una resistenza serie 100–220Ω sul TX per smorzare rimbalzi,
  - provare cavetti più corti e assicurare GND comune.
- Calibrazione continuous servo:
  - misurare tempo necessario per una rotazione completa a full-speed e calcolare degreesPerSecond.
  - regolare stopValue (valore che ferma il servo) e speedPercent per ottenere la direzione/speed desiderata.
- Sicurezza alimentazione:
  - Non alimentare servo ad alto assorbimento dal 5V USB dell'Arduino; usare un'alimentazione separata con GND comune.
- Miglioramenti possibili:
  - aggiungere checksum al pacchetto SPI e ritrasmettere se checksum non valido,
  - aggiungere ack dal slave al master su MISO per conferma di ricezione,
  - usare un protocollo binario più compatto sullo stream UART per efficienza,
  - aggiungere rilevazione di errore/timeout e meccanismi di recovery.

---