#include <SD_MMC.h>

int i = 0;

void setup(){
  // put your setup code here, to run once:
  Serial.begin(115200);
  if(!SD_MMC.begin()){
    Serial.println("Errore: SD non inizializzata!");
    return;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  loadJson();
  i++;
  delay(1000);
}

void loadJson(){
  File dati = SD_MMC.open("/dati.json",FILE_WRITE);
  if(dati){
    dati.print("{\"counter\":");
    dati.print(i);
    dati.println("}");
    dati.close();
    Serial.println("File scritto!");
  }
}
