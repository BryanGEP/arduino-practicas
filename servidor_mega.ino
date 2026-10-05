/*
  PRÁCTICA No.22 - SERVIDOR
  Arduino Mega + WiFi Shield oficial
  ===================================
  Pines:
    Pin 6  → Servo X (señal)
    Pin 7  → Servo Y (señal)
    Pin 8  → Láser KY-008 (S)
    5V     → VCC servos y láser
    GND    → GND todo

  Protocolo recibido:
    GET /?x=90&y=90&laser=1
*/

#include <SPI.h>
#include <WiFi.h>
#include <Servo.h>

// ── Tu red WiFi ────────────────────────────────────────────────
char ssid[]     = "iPhone de Brian";
char password[] = "12345678";   // 
// ───────────────────────────────────────────────────────────────

// Pines
#define PIN_SERVO_X  6
#define PIN_SERVO_Y  7
#define PIN_LASER    8

Servo servoX;
Servo servoY;

int status = WL_IDLE_STATUS;
WiFiServer server(80);

int posX    = 90;
int posY    = 90;
bool laserOn = false;

// ── Helper: extrae valor de parámetro en URL ───────────────────
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
  Serial.begin(9600);

  // Servos
  servoX.attach(PIN_SERVO_X);
  servoY.attach(PIN_SERVO_Y);
  servoX.write(posX);
  servoY.write(posY);

  // Láser
  pinMode(PIN_LASER, OUTPUT);
  digitalWrite(PIN_LASER, LOW);

  // WiFi
  Serial.print("Conectando a: ");
  Serial.println(ssid);

  // Si tu red no tiene contraseña usa: WiFi.begin(ssid)
  status = WiFi.begin(ssid, password);

  if (status != WL_CONNECTED) {
    Serial.println("Error conectando al WiFi");
    while (true); // detiene si no conecta
  }

  Serial.println("WiFi conectado!");
  Serial.print("IP del servidor (tu Arduino): ");
  Serial.println(WiFi.localIP());  // <-- dale esta IP a tu compañero

  server.begin();
  Serial.println("Servidor HTTP iniciado en puerto 80");
  Serial.println("Esperando comandos...");
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  // Espera datos
  unsigned long t = millis();
  while (!client.available() && millis() - t < 1000) delay(1);

  // Lee primera línea: GET /?x=90&y=90&laser=1 HTTP/1.1
  String requestLine = client.readStringUntil('\n');
  requestLine.trim();
  Serial.println("Recibido: " + requestLine);

  // Vacía headers
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }

  // Parsea parámetros
  int xVal     = getParam(requestLine, "x");
  int yVal     = getParam(requestLine, "y");
  int laserVal = getParam(requestLine, "laser");

  if (xVal >= 0 && xVal <= 180) {
    posX = xVal;
    servoX.write(posX);
  }
  if (yVal >= 0 && yVal <= 180) {
    posY = yVal;
    servoY.write(posY);
  }
  if (laserVal == 0 || laserVal == 1) {
    laserOn = (laserVal == 1);
    digitalWrite(PIN_LASER, laserOn ? HIGH : LOW);
  }

  Serial.print("X:"); Serial.print(posX);
  Serial.print(" Y:"); Serial.print(posY);
  Serial.print(" Laser:"); Serial.println(laserOn ? "ON" : "OFF");

  // Respuesta JSON al cliente
  String json = "{\"x\":" + String(posX) +
                ",\"y\":" + String(posY) +
                ",\"laser\":" + String(laserOn ? 1 : 0) + "}";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();
  client.println(json);

  delay(1);
  client.stop();
}
