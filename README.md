
# Relazioni di laboratorio

Questa repository raccoglie le relazioni e i sorgenti degli esperimenti con Arduino svolti durante le lezioni. Lo scopo è tenere traccia della documentazione, dei codici (.ino) e dei materiali collegati ai progetti svolti in classe.

## Struttura
- `Progetto 3 protocolli/` — relazioni e sketch per i progetti di protocolli
	- `Arduino UNO/`
	- `Arduino UNO R3/`
	- `ESP32/`

## Come usare
- Aggiungi qui una breve relazione in formato Markdown per ogni esperimento.
- Metti i file di codice (sketch `.ino`) nelle cartelle corrispondenti.
- Includi eventuali note, diagrammi o riferimenti nella relazione.

Questo file fornisce un punto d'ingresso rapido per orientarsi nella repository.

## Conversione in Word (.docx)

È presente uno script `convert_to_docx.bat` nella root che automatizza la conversione dei file Markdown in `.docx` usando Pandoc (se è presente in `PATH` o nella cartella `pandoc-3.8.2.1`).

- Esempio rapido (usa lo script, default style di highlighting `tango`):
	- `convert_to_docx.bat "Progetto 3 protocolli\RELAZIONE_PROGETTO.md"`
- Se vuoi chiamare Pandoc direttamente (con syntax highlighting esplicito):
	- `pandoc "Progetto 3 protocolli\RELAZIONE_PROGETTO.md" -o "Progetto 3 protocolli\RELAZIONE_PROGETTO.docx" --syntax-highlighting=tango`
- Per usare il Pandoc incluso nella cartella del repository:
	- `"%CD%\\pandoc-3.8.2.1\\pandoc.exe" "Progetto 3 protocolli\\RELAZIONE_PROGETTO.md" -o "Progetto 3 protocolli\\RELAZIONE_PROGETTO.docx" --syntax-highlighting=tango`

Note:
- Assicurati che i blocchi di codice nel Markdown siano fenced e abbiano l'identificatore di linguaggio (es. ```````cpp```) per ottenere il corretto highlighting.
- Se vuoi controllare font e stili nel file `.docx`, crea un modello Word `reference.docx` con gli stili desiderati e passa l'opzione `--reference-doc="reference.docx"` a Pandoc.

