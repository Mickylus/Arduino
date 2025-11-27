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
