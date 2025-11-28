# Umiditech

Breve documentazione per il progetto Umiditech — lettura umidità/temperatura con DHT11, display I2C, buzzer e interfaccia web su ESP32.

**Contenuto**
- `UmiditechV1/` : versione originale per Arduino UNO R4 WiFi (se presente).
- `UmiditechV2/` : versione per ESP32 Wrover (`UmiditechV2.ino`).
- `web/` : risorse web (HTML, JS, CSS, `wifi.json` di esempio).

**Hardware (essenziale)**
- **Sensore**: DHT11 (pin dati su `DHTPIN`, default GPIO15 nella versione ESP32).
- **Display**: LCD I2C 16x2 (indirizzo `0x27`, SDA=21, SCL=22 su ESP32).
- **Buzzer**: pin PWM (`BUZZ`, default GPIO27 su ESP32).
- **Potenziometro**: collegato a un ADC (default `POT_PIN` GPIO34 su ESP32).
- **LED**: optional, `LED` (GPIO2) per stato.

**Wiring veloce (ESP32)**
- DHT11 data -> `GPIO15` (modificabile in codice)
- LCD SDA -> `GPIO21`, SCL -> `GPIO22`, VCC -> 5V/3.3V (secondo modulo), GND -> GND
- Buzzer -> `GPIO27` (o altro pin PWM), GND -> GND
- Potenziometro -> `GPIO34` (centrale al pin analogico), estremi a 3.3V e GND

**Schema wiring (ASCII)**

Questo schema mostra i collegamenti principali per l'ESP32 Wrover e i componenti del progetto.

```
          ESP32 Wrover
         ---------------------
        |  USB |  EN     IO2 |-- LED (optional)
        |      |         IO15|-- DHT11 DATA
        |      |         IO21|-- LCD SDA (I2C)
        |      |         IO22|-- LCD SCL (I2C)
        |      |         IO27|-- Buzzer (+)
        |      |         GND |-- Buzzer (-), LCD GND, DHT GND
        |      |         3V3 |-- LCD VCC (3.3V) , DHT VCC
        |      |         IO34|-- Potenziometro (centrale)
         ---------------------

  DHT11: VCC -> 3.3V    DATA -> IO15    GND -> GND

  LCD I2C: VCC -> 3.3V   SDA -> IO21   SCL -> IO22   GND -> GND

  Buzzer (passivo o attivo): + -> IO27    - -> GND

  Potenziometro: VCC -> 3.3V  OUT -> IO34  GND -> GND

```

**Legenda e note**
- Usa 3.3V per alimentare i dispositivi collegati all'ESP32 (non 5V) a meno che il componente sia specificato per 5V.
- Se il tuo modulo LCD richiede 5V, assicurati che il bus I2C sia compatibile o usa un livello logico adeguato.
- Il buzzer può essere collegato direttamente a un pin PWM se è a bassa corrente; per buzzer che richiedono più corrente, usa un transistor e un'alimentazione separata.
- Il pin del potenziometro deve essere collegato a ADC1 (es. IO34) per evitare conflitti di ADC.

**Aggiunta lettore MicroSD (opzionale)**

Se aggiungi un modulo lettore MicroSD al progetto per salvare log o configurazioni, tieni conto di quanto segue:

- Pin SPI consigliati (ESP32, VSPI): `MOSI=GPIO23`, `MISO=GPIO19`, `SCK=GPIO18`. Il pin `CS` (chip select) può essere qualsiasi GPIO libero (es. `GPIO5`). Verifica i pin specifici della tua board.
- Alimentazione: il modulo SD deve essere alimentato a `3.3V`. Alcuni moduli hanno convertitori di livello integrati; se il modulo non è 3.3V-safe, usa un adattatore di livello.
- Libreria: usa la libreria `SD.h` (inclusa nel core ESP32) o `SD_MMC` a seconda del modulo; per un lettore SPI usa `SD.h`.
- Inizializzazione (esempio):

```cpp
#include <SD.h>
const int SD_CS = 5; // scegli il pin CS
if(!SD.begin(SD_CS)){
  Serial.println("SD init failed");
} else {
  Serial.println("SD mounted");
}
File f = SD.open("/log.txt", FILE_APPEND);
if(f){ f.println("test entry"); f.close(); }
```

- Conflitti SPI: SD usa il bus SPI; se hai altri dispositivi SPI (es. sensori o display che non sono I2C), assicurati di usare pin CS distinti e di gestire correttamente lo stato dei CS. I2C (LCD) usa SDA/SCL e non confligge con SPI.
- File system: i file saranno accessibili in `/` quando montati; non confondere SPIFFS/LittleFS (memoria interna) con la SD (scheda esterna). Se prevedi di servire i file web dalla SD usa i percorsi corretti.

Esempi di utilizzo utile:
- Salvare dati di sensori (`dati.json` o log) sulla SD per download fisico.
- Caricare configurazioni o firmware da file sulla SD (avanzato).

Nota: verifica sempre i pin e il wiring della tua specifica board ESP32 Wrover: alcune board hanno pin SPI diversi o riservati.

**Dipendenze software**
- Librerie Arduino richieste (installa da Library Manager): `DHT sensor library`, `LiquidCrystal_I2C` (o compatibile), `WiFi` (inclusa core ESP32).

**Installazione e upload (ESP32)**
- Se usi l'IDE Arduino:
  - Seleziona scheda `ESP32 Wrover Module` e porta corretta.
  - Installa le librerie richieste.
  - Apri `UmiditechV2/UmiditechV2.ino` e caricalo.
- Se usi PlatformIO: crea un progetto con board `esp32-wrover-module` e carica lo sketch.

**SPIFFS / File web**
- Lo sketch `UmiditechV2.ino` usa SPIFFS per salvare `/wifi.json` (POST handler) e può servire file web se li carichi su SPIFFS.
- Per caricare i file web su SPIFFS con l'IDE Arduino, usa il plugin `ESP32FS` (o lo strumento `PlatformIO: Upload File System`).

**Uso della pagina di impostazioni**
- Apri `settings.html` (se servita dall'ESP, punta al `http://<IP_ESP>/settings.html`).
- Inserisci `SSID` e `Password` e premi `Salva`:
  - Il browser prova a fare `POST /wifi.json` sull'ESP; se il server ESP è disponibile, il file viene salvato su SPIFFS.
  - Se il server non è disponibile, lo script JS salva i valori in `localStorage` e avvia il download automatico di `wifi.json` sul PC (fallback).

**Esempio: test POST con `curl` (Windows `cmd.exe`)**
```batch
curl -X POST -H "Content-Type: application/json" --data "{\"ssid\":\"UmidiTech\",\"password\":\"12345678\"}" http://192.168.4.1/wifi.json
```

**Note operative**
- Dopo aver salvato `wifi.json` su SPIFFS, lo sketch può leggerlo e, se desideri, connettersi come station (client). In questa implementazione il device crea un AP e serve la pagina.
- Se vuoi che la web UI sia raggiungibile dall'ESP senza server esterno, carica la cartella `web/` su SPIFFS e aggiungi gli handler statici (posso aiutarti a farlo).

**Prossimi passi suggeriti**
- Caricare i file web su SPIFFS e servire la UI direttamente dall'ESP.
- Aggiungere validazione/feedback maggiori nella UI (es. gestione errori lettura DHT).

---

Se vuoi, aggiorno subito questo file aggiungendo uno schema wiring in ASCII o generando l'elenco preciso dei file da caricare su SPIFFS. Dimmi cosa preferisci.
