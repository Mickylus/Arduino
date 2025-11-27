#include <SD.h>
#include <SPI.h>

#define SD_CS   5
#define SD_SCK  14
#define SD_MISO 2
#define SD_MOSI 15

void setup() {
  Serial.begin(115200);
  delay(2000);

  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS)) {
    Serial.println("Errore: SD non inizializzata!");
    return;
  }

  Serial.println("SD inizializzata!");

  File f = SD.open("/test.txt", FILE_WRITE);
  if(!f){
    Serial.println("Impossibile aprire file");
    return;
  }
  f.println("Ciao!");
  f.close();
}

void loop() {}
