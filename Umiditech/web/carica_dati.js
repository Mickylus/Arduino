async function caricaDati(){
	try{
		const risposta = await fetch("../dati.json");
		const dati = await risposta.json();

		const elH = document.getElementById("humidity");
		const elT = document.getElementById("temperature");
		const elV = document.getElementById("volume");
		if(elH) elH.textContent = (dati.umidita !== undefined ? dati.umidita : '');
		if(elT) elT.textContent = (dati.temperatura !== undefined ? dati.temperatura : '');
		if(elV) elV.textContent = (dati.volume !== undefined ? dati.volume : '');


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
		const valEl = document.getElementById("valore");
		if(valEl) valEl.textContent = "Errore nel caricamento dati";
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

// --- WiFi settings: load + save handlers ----------------------------------
async function caricaWifi(){
	try{
		// prova a leggere wifi.json (server mappa /wifi.json -> /web/wifi.json)
		const resp = await fetch('wifi.json');
		if(!resp.ok) return; // nulla da fare
		const w = await resp.json();
		if(w.ssid) document.getElementById('ssid').value = w.ssid;
		if(w.password) document.getElementById('password').value = w.password;
	}catch(e){
		console.warn('caricaWifi failed', e);
	}
}

async function salvaWifi(evt){
	if(evt && evt.preventDefault) evt.preventDefault();
	const ssid = (document.getElementById('ssid')||{}).value || '';
	const password = (document.getElementById('password')||{}).value || '';
	const payload = { ssid: ssid, password: password };
	const statusEl = document.getElementById('saveStatus');
	if(statusEl) statusEl.textContent = 'Salvataggio...';

	// Prova POST al server (ESP) su /wifi.json
	try{
		const resp = await fetch('/wifi.json', {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(payload)
		});
		if(resp.ok){
			if(statusEl) statusEl.textContent = 'Salvato sul dispositivo';
			return;
		}
		// se risponde ma non ok, fallback
		console.warn('Server responded', resp.status);
	}catch(err){
		console.warn('POST /wifi.json failed', err);
	}

	// Fallback: salva localmente come download (l'utente poi copia su SD)
	try{
		const blob = new Blob([JSON.stringify(payload, null, 2)], { type: 'application/json' });
		const url = URL.createObjectURL(blob);
		const a = document.createElement('a');
		a.href = url;
		a.download = 'wifi.json';
		document.body.appendChild(a);
		a.click();
		a.remove();
		URL.revokeObjectURL(url);
		if(statusEl) statusEl.textContent = 'Download wifi.json creato (salvalo su SD)';
		// Also store in localStorage as convenience
		try{ localStorage.setItem('wifi.json', JSON.stringify(payload)); }catch(_){}
	}catch(e){
		console.error('Fallback save failed', e);
		if(statusEl) statusEl.textContent = 'Errore nel salvataggio';
	}
}

// Hook form on load
try{
	// If DOM already loaded, attach immediately, otherwise wait
	const attach = ()=>{
		const form = document.getElementById('settingsBox');
		if(form) form.addEventListener('submit', salvaWifi);
		caricaWifi();
	};
	if(document.readyState === 'loading') document.addEventListener('DOMContentLoaded', attach);
	else attach();
}catch(e){ console.warn('Could not attach settings handlers', e); }