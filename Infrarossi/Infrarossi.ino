#include <IRremote.hpp>

// Pin per il ricevitore IR
const int IR_RECEIVE_PIN = 12;

void setup() {
  Serial.begin(9600); // se usi una board che richiede il wait, puoi usare while (!Serial);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Avvia il ricevitore IR
}

void loop() {
  // Verifica se è arrivato un segnale IR decodificato
  if (IrReceiver.decode()) {
    unsigned long cmd = IrReceiver.decodedIRData.command;
    Serial.print("Mapped key: ");
    Serial.println(decodeKeyValue(cmd)); // stampa la mappatura leggibile

    IrReceiver.resume(); // abilita la ricezione del prossimo valore
  }

  delay(1000); // opzionale, evita flood della seriale
}
//Sistemando il codiece efeffef efef effeef efeafaoijiojoi
// Mappa i comandi IR ai nomi dei tasti
const char* decodeKeyValue(unsigned long result) {
  switch (result) {
    case 69UL: return "POWER";
    case 71UL: return "MENU";
    case 68UL: return "TEST";
    case 64UL: return "+"; 
    case 67UL: return "Back";
    case 7UL:  return "|<<";
    case 21UL: return "▶";
    case 9UL:  return ">>|";
    case 22UL: return "0";
    case 25UL: return "-";
    case 13UL: return "C";
    case 12UL: return "1";
    case 24UL: return "2";
    case 94UL: return "3";
    case 8UL:  return "4";
    case 28UL: return "5";
    case 90UL: return "6";
    case 66UL: return "7";
    case 82UL: return "8";
    case 74UL: return "9";
    default:   return "UNKNOWN";
  }
}