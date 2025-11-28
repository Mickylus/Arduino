async function caricaDati(){
	try{
		const risposta = await fetch("dati.json");
		const dati = await risposta.json();

		document.getElementById("humidity").textContent = dati.umidita
	}catch(errore){
		console.error("Errore nel leggere il JSON:", errore);
        document.getElementById("valore").textContent = "Errore nel caricamento dati";
	}
}

caricaDati();


// Salva SSID e password in un file JSON sul server (POST a "wifi.json").
async function salvaWifi(){
	const statusEl = document.getElementById('saveStatus');
	try{
		const ssid = document.getElementById('ssid').value || "";
		const password = document.getElementById('password').value || "";

		const payload = { ssid: ssid, password: password };

		statusEl.textContent = 'Salvataggio...';

		const resp = await fetch('wifi.json', {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(payload)
		});

		if(!resp.ok){
			throw new Error('HTTP ' + resp.status);
		}

		statusEl.textContent = 'Salvataggio riuscito';
		setTimeout(()=> statusEl.textContent = '', 3000);
	}catch(err){
		console.error('Errore salvataggio wifi:', err);
		// Fallback: il server non salva i file via POST -> salvataggio lato client
		try{
			const ssid = document.getElementById('ssid').value || "";
			const password = document.getElementById('password').value || "";
			const payload = { ssid: ssid, password: password };

			// Salva in localStorage
			try{ localStorage.setItem('wifi', JSON.stringify(payload)); }catch(e){}

			// Crea file e avvia download: wifi.json
			const blob = new Blob([JSON.stringify(payload, null, 2)], {type: 'application/json'});
			const url = URL.createObjectURL(blob);
			const a = document.createElement('a');
			a.href = url;
			a.download = 'wifi.json';
			document.body.appendChild(a);
			a.click();
			a.remove();
			URL.revokeObjectURL(url);

			if(statusEl) statusEl.textContent = 'Server non disponibile: file scaricato localmente';
			setTimeout(()=> statusEl.textContent = '', 5000);
		}catch(e){
			if(statusEl) statusEl.textContent = 'Errore nel salvataggio';
		}
	}
}

// Riempi il form con i valori esistenti (se presente il file wifi.json)
async function caricaWifi(){
	try{
		const r = await fetch('wifi.json');
		if(!r.ok) return;
		const j = await r.json();
		if(j.ssid) document.getElementById('ssid').value = j.ssid;
		if(j.password) document.getElementById('password').value = j.password;
	}catch(e){
		// silenzioso
	}
}

// Se la pagina contiene il form, carica i valori
if(document.getElementById('ssid')){
	caricaWifi();
}