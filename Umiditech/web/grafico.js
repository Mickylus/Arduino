// grafico.js
// Carica i dati da ../dati.json e disegna un grafico usando Chart.js
(function(){
	let chart = null;
	function creaGrafico(){
		const canvas = document.getElementById('grafico');
		if(!canvas) return null;
		const ctx = canvas.getContext('2d');
		// Default dataset iniziale
		const data = {
			labels: ['Umidità', 'Temperatura', 'Volume'],
			datasets: [{
				label: 'Valore',
				data: [0,0,0],
				backgroundColor: ['#4e79a7','#f28e2b','#e15759'],
				borderColor: ['#2b4f77','#a55a19','#8b2b2b'],
				borderWidth: 1
			}]
		};

		const config = {
			type: 'bar',
			data: data,
			options: {
				responsive: true,
				maintainAspectRatio: false,
				scales: {
					y: {
						beginAtZero: true
					}
				},
				plugins: {
					legend: { display: false }
				}
			}
		};

		return new Chart(ctx, config);
	}

	async function aggiornaDati(){
		try{
			const resp = await fetch('../dati.json', { cache: 'no-store' });
			if(!resp.ok) return;
			const dati = await resp.json();
			const um = Number(dati.umidita || 0);
			const te = Number(dati.temperatura || 0);
			const vo = Number(dati.volume || 0);
			if(chart && chart.data && chart.data.datasets && chart.data.datasets[0]){
				chart.data.datasets[0].data = [um, te, vo];
				chart.update();
			}
		}catch(e){
			console.error('Errore aggiornamento grafico:', e);
		}
	}

	function start(){
		chart = creaGrafico();
		// Primo aggiornamento immediato
		aggiornaDati();
		// Aggiorna ogni 1s (non troppo veloce per evitare sovraccarico)
		setInterval(aggiornaDati, 1000);
	}

	if(document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start);
	else start();
})();
