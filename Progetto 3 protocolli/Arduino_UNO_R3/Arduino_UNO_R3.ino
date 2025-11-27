/*
  Arduino UNO R3 - Slave
  - Riceve 3 byte via Serial: X, Y, B
  - Muove il servo (pin 3) usando X e mostra X,Y,B sul display I2C
*/

#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myservo;

void setup() {
  Serial.begin(115200);
  myservo.attach(3);
  myservo.write(90); // posizione iniziale

  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Ready (Serial)");
}

void loop() {
  // Aspetta di avere almeno 3 byte: X, Y, B
  if (Serial.available() >= 3) {
    int x = Serial.read();
    int y = Serial.read();
    int b = Serial.read();

    // Semplice validazione di X
    if(x < 0){
      x = 0;
    }
    if(x > 180){
      x = 180;
    }
    myservo.write(x);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("X:"); lcd.print(x);
    lcd.print(" Y:"); lcd.print(y);
    lcd.setCursor(0,1);
    lcd.print("B:"); lcd.print((b != 0) ? 1 : 0);
    Serial.print("Got: "); Serial.print(x); Serial.print(","); Serial.print(y); Serial.print(","); Serial.println(b);
  }
  delay(10);
}
