async function caricaDati(){
	try{
		const risposta = await fetch("dati.json");
		const dati = await risposta.json();

		document.getElementById("humidity").textContent = dati.umidita;
		document.getElementById("temperature").textContent = dati.temperatura;
		document.getElementById("volume").textContent = dati.volume;


		// Applica classi di stato ai valori invece di inline style (stessa logica soglie)
		function applyStatusClass(el, className){
			if(!el) return;
			el.classList.remove('status-red','status-amber','status-green','status-dark');
			if(className) el.classList.add(className);
		}

		function setColorByValue(el, value, kind){
			if(!el) return;
			const v = Number(value);
			if(isNaN(v)) { applyStatusClass(el, null); return; }
			if(kind === 'humidity'){
				if(v >= 65) applyStatusClass(el, 'status-red'); // rosso
				else if(v >= 55) applyStatusClass(el, 'status-amber'); // arancione
				else applyStatusClass(el, 'status-green'); // verde scuro
			} else if(kind === 'temperature'){
				if(v >= 30) applyStatusClass(el, 'status-red');
				else if(v >= 25) applyStatusClass(el, 'status-amber');
				else applyStatusClass(el, 'status-green');
			} else if(kind === 'volume'){
				if(v >= 80) applyStatusClass(el, 'status-red');
				else if(v >= 50) applyStatusClass(el, 'status-amber');
				else applyStatusClass(el, 'status-dark');
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