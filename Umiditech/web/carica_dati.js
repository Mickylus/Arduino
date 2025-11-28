async function caricaDati(){
	try{
		const risposta = await fetch("dati.json");
		const dati = await risposta.json();

		document.getElementById("humidity").textContent = dati.umidita;
		document.getElementById("temperature").textContent = dati.temperatura;
		document.getElementById("volume").textContent = dati.volume;

		// Colora il testo in base ai valori
		function setColorByValue(el, value, kind){
			if(!el) return;
			const v = Number(value);
			if(isNaN(v)) { el.style.color = ''; return; }
			if(kind === 'humidity'){
				if(v >= 65) el.style.color = '#b22222'; // rosso
				else if(v >= 55) el.style.color = '#ff8c00'; // arancione
				else el.style.color = '#006400'; // verde scuro
			} else if(kind === 'temperature'){
				if(v >= 30) el.style.color = '#b22222';
				else if(v >= 25) el.style.color = '#ff8c00';
				else el.style.color = '#006400';
			} else if(kind === 'volume'){
				if(v >= 80) el.style.color = '#b22222';
				else if(v >= 50) el.style.color = '#ff8c00';
				else el.style.color = '#000000';
			}
		}

		setColorByValue(document.getElementById('humidity'), dati.umidita, 'humidity');
		setColorByValue(document.getElementById('temperature'), dati.temperatura, 'temperature');
		setColorByValue(document.getElementById('volume'), dati.volume, 'volume');
	}catch(errore){
		console.error("Errore nel leggere il JSON:", errore);
        document.getElementById("valore").textContent = "Errore nel caricamento dati";
	}
}

async function aggiorna() {
	while(true){
		await caricaDati();
		await new Promise(r => setTimeout(r,200));
	}
}

// Codice esguito
aggiorna();