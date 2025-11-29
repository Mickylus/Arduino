// Clean ESP32 test sketch
// - Mounts SD (SPI)
// - Reads Wi-Fi credentials from /web/wifi.json or /wifi.json on SD
// - Connects as STA when credentials are present
// - Serves files from /web via HTTP
// - Periodically writes /web/dati.json with { "umidita", "temperatura", "volume" }
// - Accepts POST /wifi.json (with CORS preflight) and writes to /web/wifi.json

#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <SD.h>

#define MOSI_SD 15
#define MISO_SD 2
#define SCK_SD 14
#define CS_SD 5

WebServer server(80);

unsigned long lastWrite = 0;
const unsigned long WRITE_INTERVAL_MS = 2000; // write dati.json every 2s

// Synthetic sensor values
int umidita = 34;
int temperatura = 23;
int volumeVal = 60;

String contentType(const String &filename) {
    if (filename.endsWith(".htm") || filename.endsWith(".html")) return "text/html";
    if (filename.endsWith(".css")) return "text/css";
    if (filename.endsWith(".js")) return "application/javascript";
    if (filename.endsWith(".json")) return "application/json";
    if (filename.endsWith(".png")) return "image/png";
    if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg";
    if (filename.endsWith(".ico")) return "image/x-icon";
    return "text/plain";
}

// Serve a file from SD under /web
bool serveFromSD(const String &uri) {
    String path = uri;

    if (path == "/")
        path = "/web/index.html";
    else if (!path.startsWith("/web/"))
        path = "/web" + path;

    if (!SD.exists(path)) return false;

    File file = SD.open(path, FILE_READ);
    if (!file) return false;

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "no-store, max-age=0");

    server.streamFile(file, contentType(path));
    file.close();

    Serial.printf("Served %s\n", path.c_str());
    return true;
}

void handleNotFound() {
    if (serveFromSD(server.uri())) return;

    String message = "File Not Found\nURI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "OTHER";
    server.send(404, "text/plain", message);
}

void handleWifiOptions() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204, "text/plain", "");
}

bool sdAtomicWrite(const char *path, const String &data) {
    String tmp = String(path) + ".tmp";

    if (SD.exists(tmp.c_str())) SD.remove(tmp.c_str());
    File f = SD.open(tmp.c_str(), FILE_WRITE);

    if (!f) {
        Serial.printf("Failed to open tmp file %s\n", tmp.c_str());
        return false;
    }

    size_t written = f.print(data);
    f.close();

    if (written == 0) {
        Serial.println("Nothing written to temp file");
        SD.remove(tmp.c_str());
        return false;
    }

    if (SD.exists(path)) SD.remove(path);
    bool ok = SD.rename(tmp.c_str(), path);

    if (!ok) {
        Serial.printf("Rename failed: %s -> %s\n", tmp.c_str(), path);

        File t = SD.open(tmp.c_str(), FILE_READ);
        File d = SD.open(path, FILE_WRITE);
        if (!t || !d) {
            if (t) t.close();
            if (d) d.close();
            SD.remove(tmp.c_str());
            return false;
        }

        uint8_t buf[128];
        while (t.available()) {
            size_t r = t.read(buf, sizeof(buf));
            d.write(buf, r);
        }

        t.close();
        d.close();
        SD.remove(tmp.c_str());
        return true;
    }

    return true;
}

void handleWifiPost() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing body\"}");
        return;
    }

    String body = server.arg("plain");
    Serial.printf("Received /wifi.json body: %s\n", body.c_str());

    bool wrote = sdAtomicWrite("/web/wifi.json", body);
    if (wrote) {
        server.send(200, "application/json", "{\"ok\":true}");
        Serial.println("Saved /web/wifi.json");
    } else {
        server.send(500, "application/json", "{\"ok\":false,\"error\":\"write failed\"}");
    }
}

void connectWiFiFromSD() {
    const char *paths[] = {"/web/wifi.json", "/wifi.json"};
    String ssid = "", pass = "";

    for (auto p : paths) {
        if (!SD.exists(p)) continue;
        File f = SD.open(p, FILE_READ);
        if (!f) continue;

        String content;
        while (f.available()) content += (char)f.read();
        f.close();

        int spos = content.indexOf("\"ssid\"");
        if (spos >= 0) {
            int colon = content.indexOf(':', spos);
            int q1 = content.indexOf('"', colon);
            int q2 = content.indexOf('"', q1 + 1);
            if (q2 > q1) ssid = content.substring(q1 + 1, q2);
        }

        int ppos = content.indexOf("\"password\"");
        if (ppos >= 0) {
            int colon = content.indexOf(':', ppos);
            int q1 = content.indexOf('"', colon);
            int q2 = content.indexOf('"', q1 + 1);
            if (q2 > q1) pass = content.substring(q1 + 1, q2);
        }

        if (ssid.length()) break;
    }

    if (ssid.length() == 0) {
        Serial.println("No WiFi credentials on SD, not starting STA.");
        return;
    }

    Serial.printf("Connecting to SSID: %s\n", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(200);
        Serial.print('.');
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
        Serial.printf("Connected. IP: %s\n", WiFi.localIP().toString().c_str());
    else
        Serial.println("Failed to connect to WiFi");
}

void aggiornaDati() {
    umidita += 1;
    temperatura += 1;

    if (umidita > 100) umidita = 0;
    if (temperatura > 100) temperatura = 0;

    String json = "{";
    json += "\"umidita\":" + String(umidita) + ",";
    json += "\"temperatura\":" + String(temperatura) + ",";
    json += "\"volume\":" + String(volumeVal);
    json += "}";

    bool ok = sdAtomicWrite("/web/dati.json", json);
    if (ok) Serial.println("Wrote /web/dati.json");
    else Serial.println("Failed to write /web/dati.json");
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("Starting test.ino");

    SPI.begin(SCK_SD, MISO_SD, MOSI_SD);
    if (!SD.begin(CS_SD, SPI)) {
        Serial.println("SD init failed. Check wiring and CS pin.");
    } else {
        Serial.println("SD initialized.");
    }

    connectWiFiFromSD();

    server.on("/wifi.json", HTTP_OPTIONS, handleWifiOptions);
    server.on("/wifi.json", HTTP_POST, handleWifiPost);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");

    if (!SD.exists("/web/dati.json")) {
        aggiornaDati();
    }
}

void loop() {
    server.handleClient();

    unsigned long now = millis();
    if (now - lastWrite >= WRITE_INTERVAL_MS) {
        lastWrite = now;
        aggiornaDati();
    }
}
