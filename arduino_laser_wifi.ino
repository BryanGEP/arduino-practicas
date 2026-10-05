/*
  PRÁCTICA No.22 - Arduino WiFi - Control Láser con Servomotores
  ---------------------------------------------------------------
  Hardware necesario:
    - Arduino Uno + Shield WiFi (ESP8266 o similar)   ← opción A
    - Arduino Mega + Shield WiFi                       ← opción B
    - NodeMCU / Wemos D1 (ESP8266 nativo)              ← opción C (recomendada)
  
  Este sketch asume NodeMCU / Wemos D1 Mini (ESP8266).
  Si usas Arduino Uno + Shield ESP8266 cambia los pines de Serial.
  
  Pines:
    D5  → Señal Servo X (eje horizontal)
    D6  → Señal Servo Y (eje vertical)
    D7  → Control LED Láser (HIGH = encendido)
  
  Protocolo recibido por TCP (puerto 80 - HTTP):
    GET /?x=90&y=90&laser=1   → mueve servos y controla láser
*/

#include <Wifi.h>
#include <Servo.h>

// ── Configura aquí tu red WiFi ─────────────────────────────────
const char* ssid     = "ALUMNOSR1";
const char* password = "00000000";
// ───────────────────────────────────────────────────────────────

// Pines
#define PIN_SERVO_X  D5
#define PIN_SERVO_Y  D6
#define PIN_LASER    D7

Servo servoX;
Servo servoY;

WiFiServer server(80);

// Posiciones actuales
int posX = 90;
int posY = 90;
bool laserOn = false;

// ── Helpers ────────────────────────────────────────────────────

// Extrae el valor de un parámetro de una URL tipo ?x=90&y=45&laser=1
int getParam(String url, String param) {
  String search = param + "=";
  int idx = url.indexOf(search);
  if (idx == -1) return -1;
  idx += search.length();
  int end = url.indexOf('&', idx);
  if (end == -1) end = url.length();
  return url.substring(idx, end).toInt();
}

void setup() {
  Serial.begin(115200);

  // Servos
  servoX.attach(PIN_SERVO_X);
  servoY.attach(PIN_SERVO_Y);
  servoX.write(posX);
  servoY.write(posY);

  // Láser
  pinMode(PIN_LASER, OUTPUT);
  digitalWrite(PIN_LASER, LOW);

  // WiFi
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP del Arduino: ");
  Serial.println(WiFi.localIP());  // <-- apunta esta IP en server.py

  server.begin();
  Serial.println("Servidor HTTP iniciado en puerto 80");
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  // Espera datos del cliente
  unsigned long timeout = millis() + 1000;
  while (!client.available() && millis() < timeout) delay(1);

  // Lee la primera línea (GET /...  HTTP/1.1)
  String requestLine = client.readStringUntil('\n');
  requestLine.trim();
  Serial.println("Request: " + requestLine);

  // Vacía el resto de headers
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }

  // Parsea parámetros
  // Ejemplo: GET /?x=90&y=45&laser=1 HTTP/1.1
  int xVal     = getParam(requestLine, "x");
  int yVal     = getParam(requestLine, "y");
  int laserVal = getParam(requestLine, "laser");

  bool changed = false;

  if (xVal >= 0 && xVal <= 180) {
    posX = xVal;
    servoX.write(posX);
    changed = true;
  }
  if (yVal >= 0 && yVal <= 180) {
    posY = yVal;
    servoY.write(posY);
    changed = true;
  }
  if (laserVal == 0 || laserVal == 1) {
    laserOn = (laserVal == 1);
    digitalWrite(PIN_LASER, laserOn ? HIGH : LOW);
    changed = true;
  }

  // Respuesta HTTP con estado actual en JSON
  String json = "{\"x\":" + String(posX) +
                ",\"y\":" + String(posY) +
                ",\"laser\":" + String(laserOn ? 1 : 0) + "}";

  String response = "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/json\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Content-Length: " + String(json.length()) + "\r\n"
                    "Connection: close\r\n\r\n" + json;

  client.print(response);
  client.flush();
  delay(1);
  client.stop();

  if (changed) {
    Serial.println("Estado → X:" + String(posX) +
                   " Y:" + String(posY) +
                   " Laser:" + String(laserOn ? "ON" : "OFF"));
  }
}
