
# Relazioni di laboratorio

Repository di riferimento per le relazioni e i sorgenti degli esperimenti con Arduino svolti durante le lezioni.

Scopo: mantenere documentazione, sketch (`.ino`) e materiale utile (diagrammi, note, risorse web) ordinati per esercitazione/progetto.

## Struttura della repository
Descrizione rapida dei progetti e delle cartelle principali presenti in questa repository.

- `Umiditech/` — Progetto dedicato al monitoraggio di temperatura/umidità con versioni per Arduino e per ESP32. Contiene:
	- `UmiditechV1/` — versione per Arduino UNO/UNO R4
	- `Umiditechv2/` — versione per ESP32 (Wrover) con supporto SPIFFS e web UI
	- `web/` — interfaccia web (HTML/JS/CSS) usata per visualizzare dati e impostazioni (es. `settings.html`, `carica_dati.js`, `wifi.json`)

- `Progetto 3 protocolli/` — Implementazione di un progetto che mette a confronto/integra tre protocolli di comunicazione tra schede:
	- `Arduino_UNO/` — sketch per la scheda UNO (master) che riceve da ESP32 e inoltra via SPI
	- `Arduino_UNO_R3/` — sketch per la UNO R3 (slave) che riceve comandi via SPI e controlla servo/display
	- `ESP32_sender/` — sketch per ESP32 che legge joystick/pulsante e invia comandi seriali
	- inoltre contiene la relazione `RELAZIONE_PROGETTO.md`, immagini di collegamento (`Collegamenti.jpg`) e un video di collaudo (`Collaudo.mp4`).

- `Infrarossi/` — semplici esperimenti con sensori/ricevitori infrarossi (es. `Infrarossi.ino`).

- `test/` (se presente in progetti) — directory usata per prove e file temporanei.

Ogni cartella progetto contiene generalmente:
- uno o più sketch (`*.ino`)
- una relazione in Markdown (es. `RELAZIONE_PROGETTO.md`)
- eventuali risorse: immagini, web UI, file di configurazione

## Come usare
- Aggiungi per ogni esperimento una breve relazione in Markdown che spieghi obiettivi, wiring, risultati e link a sketch e materiali.
- Metti gli sketch nella cartella del progetto corrispondente.
- Inserisci schemi di collegamento (breadboard/wiring) e note pratiche nella relazione.
