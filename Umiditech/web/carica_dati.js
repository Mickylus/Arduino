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