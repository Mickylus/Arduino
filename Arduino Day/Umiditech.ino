#include "DHT.h"                                  // Libreria per utilizzare il sensore di umidità (DTH 11)
#include <LiquidCrystal_I2C.h>                    // Libreria per utilizzare il display LCD
#include <WiFiS3.h>


// Definisco i pin dei componenti
#define DHTPIN 10
#define DHTTYPE DHT11
#define BUZZ 6
#define LED 4
#define POT A2   // Uso A0 per un input analogico a causa di problemi con gli altri pin

// Funzione della libreria che imposta il sensore
DHT dht(DHTPIN, DHTTYPE);

// Delimito le righe/colonne del display lcd
LiquidCrystal_I2C lcd(0x27, 16,2);


// Nome rete WiFi e password
char ssid[] = "UmidiTech";
char pass[] = "16321632";

WiFiServer server(80);


// Variabili:
// flag= controlla che il buzzer abbia già suonato
// mill= milliseconti attuali
// buzz= variabile per far suonare il buzzer
// milp= millisecondi corrispondenti all'ultimo controllo
// millstart= timer per il buzzer
// h,t= umidità e temperatura
int flag=0, mill, millStart, buzz=0, milp=0;
float h,t;


void setup(){
  // Inizializzo la porta seriale
  Serial.begin(9600);
  // Inizalizzo il sensore
  dht.begin();
  // Inizializzo il display
  lcd.init();
  lcd.backlight();
  lcd.print("UMIDITECH");

  Serial.println("Impostazione Access Point...");
  int status = WiFi.beginAP(ssid, pass);

  if (status != WL_AP_LISTENING) {
    Serial.println("Errore: impossibile avviare AP");
    while (true);
  }
  Serial.print("IP Arduino: ");
  Serial.println(WiFi.localIP()); // Di solito 192.168.4.1
  server.begin();
  // Inizializzo i pin
  pinMode(BUZZ,OUTPUT);
  pinMode(LED,OUTPUT);
  pinMode(POT,INPUT);
}

void loop(){
  mill=millis();
  WiFiClient client = server.available();  
  // controlla se sono passati 2000(2s) dall'ultimo controllo;
  if(mill-milp>=2000){ 
    milp=mill;    // Imposta l'ultimo controllo ai millisecondi correnti
    // leggo la temperatura e umidità
    h = dht.readHumidity();
    t = dht.readTemperature();
    // Stampo sia sulla porta seriale che sul display
    Serial.print(h);
    Serial.print(" ");
    Serial.print(t);
    Serial.print(" ");
    Serial.println(analogRead(POT));
    stampaServer(client);
    lcd.setCursor(0,1);
    lcd.print("U:");
    lcd.print(h);
    lcd.print("% T:");
    lcd.print(t);
    lcd.print("°C");
  }
  // controlla che il buzzer abbia suonato per almeno 3500(3.5s)
  if(mill - millStart >= 3500){
    analogWrite(BUZZ,0);
  }else{
    analogWrite(BUZZ,map(analogRead(POT),1,700,0,255));
  } 
  // controlla se l'umidtià abbia superato la soglia
  if(h>65){
    lcd.setCursor(0,0);
    lcd.print("CAMBIARE ARIA!! ");
    // se non ha già suonato e il volume non è al minimo avvia il contatore del buzzer
    if(flag==0 && analogRead(POT)>100){
      analogWrite(BUZZ,255);
      millStart=millis(); 
      flag=1;     // segnala che ha già suonato
    }
    digitalWrite(LED,HIGH);
  }else{
    lcd.setCursor(0,0);
    lcd.print("Regolare        ");
    digitalWrite(LED,LOW);
    analogWrite(BUZZ,0);
    flag=0;       // reimposta il segnale del buzzer
  }
}

void stampaServer(WiFiClient client){
   if(client){
    String req = client.readStringUntil('\r');
    client.flush();

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int vol = map(analogRead(POT),100,700,0,100);

    if(vol<0){
      vol=0;
    }else if(vol>100){
      vol=100;
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    client.println("<meta http-equiv='refresh' content='2'>"); // aggiorna ogni 2 secondi
    client.println("<style>");
    client.println("body { font-family: Arial; text-align: center; background: #fffff0; }");
    client.println("h1 { color: #1e90ff; font-family: Arial; font-size: 3em; font-weight:bold; }");
    client.println(".value { font-size: 2em; margin: 10px; font-family: Consolas,monaco,monospace;}");
    if(h>65){
      client.println(".value2{ font-size: 2em; margin: 10px; color: #b22222; font-family: Consolas,monaco,monospace; }");
    }else{
      client.println(".value2{ font-size: 2em; margin: 10px; color: #000000; font-family: Consolas,monaco,monospace; }");
    }
    client.println(".value3 { font-size: 2em; margin: 10px; font-family: Consolas,monaco,monospace;}");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    client.println("<h1>UMIDITECH | DATI</h1>");
    client.print("<p class='value'>Temperatura: ");
    client.print(t);
    client.println(" &deg;C</p>");
    client.print("<p class='value2'>Umidita': ");
    client.print(h);
    client.println(" %</p>");
    client.println("<p class='value3'>Volume: ");
    client.print(vol);
    client.println("%</p>");
    client.println("</body>");
    client.println("</html>");
    delay(1);
    client.stop();
  }

}