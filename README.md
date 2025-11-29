
# Relazioni di laboratorio

Repository di riferimento per le relazioni e i sorgenti degli esperimenti con Arduino svolti durante le lezioni.

Scopo: mantenere documentazione, sketch (`.ino`) e materiale utile (diagrammi, note, risorse web) ordinati per esercitazione/progetto.

## Struttura della repository
Descrizione rapida dei progetti e delle cartelle principali presenti in questa repository.

- `Umiditech/` — Progetto dedicato al monitoraggio di temperatura/umidità. Contiene:
	- `dati.json` — esempio di output/salvataggio dati
	- `README.md` — documentazione del progetto
	- `test/` — directory per prove locali con file di test (`prova.txt`, `test.ino`)
	- `UmiditechV1/` — versione per Arduino UNO/UNO R4 (sketch `UmiditechV1.ino`)
	- `Umiditechv2/` — versione per ESP32 (sketch `UmiditechV2.ino`) con supporto SPIFFS e web UI
	- `web/` — interfaccia web (HTML/JS/CSS) usata per visualizzare dati e impostazioni; esempi presenti:
		- `index.html`, `settings.html`
		- `carica_dati.js`, `chart.js`, `grafico.js`
		- `style.css`, `wifi.json`
		- `fonts/` (es. `abduction_2002/`, `panton/`)
		- `immagini/` — immagini usate nella web UI

- `Progetto 3 protocolli/` — progetto che integra/valuta più protocolli di comunicazione tra schede:
	- `Arduino_UNO/` — sketch per la scheda UNO (master) che riceve da ESP32 e inoltra via SPI (`Arduino_UNO.ino`)
	- `Arduino_UNO_R3/` — sketch per la UNO R3 (slave) che riceve comandi via SPI e controlla servo/display (`Arduino_UNO_R3.ino`)
	- `ESP32_sender/` — sketch per ESP32 che legge joystick/pulsante e invia comandi seriali (`ESP32_sender.ino`)
	- `RELAZIONE_PROGETTO.md` — relazione del progetto (collegamenti, immagini, video di collaudo)

- `Infrarossi/` — semplici esperimenti con sensori/ricevitori infrarossi (es. `Infrarossi.ino`).

- `test/` — cartelle `test/` presenti nei progetti usate per prove e file temporanei.

Ogni cartella progetto contiene generalmente:
- uno o più sketch (`*.ino`)
- una relazione in Markdown (es. `RELAZIONE_PROGETTO.md`)
- eventuali risorse: immagini, web UI, file di configurazione

## Come usare
- **Aggiungi**: per ogni esperimento inserisci una relazione in Markdown che spieghi obiettivi, wiring, risultati e link agli sketch.
- **Organizza**: metti gli sketch nella cartella del progetto corrispondente e salva immagini/schemi nella stessa cartella o in `immagini/`.
- **Web UI**: per progetti con interfaccia web (es. `Umiditech/web`) tieni insieme `index.html`, gli script JS e le risorse (`fonts/`, `immagini/`, `wifi.json`).
- **Test**: usa le directory `test/` per file temporanei e prove locali; rimuovi o archivia i contenuti importanti nel progetto principale.

Se vuoi, posso:
- aggiornare automaticamente l'elenco dei file per ogni cartella e inserirlo in questo `README.md`;
- generare uno schema dei collegamenti (immagine) basato sugli sketch disponibili.
