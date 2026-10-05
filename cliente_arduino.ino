/*
  PRÁCTICA No.22 - CLIENTE
  Arduino + WiFi Shield
  ===================================
  El compañero usa este código para enviar
  comandos al servidor (tu Arduino Mega).

  Controles por Serial Monitor:
    w → Servo Y arriba
    s → Servo Y abajo
    a → Servo X izquierda
    d → Servo X derecha
    l → Toggle láser ON/OFF
    c → Centrar servos (90,90)

  O también puede usar botones físicos,
  cambiando el código de loop().
*/

#include <SPI.h>
#include <WiFi.h>

// ── Red WiFi (la misma que el servidor) ───────────────────────
char ssid[]     = "ALUMNOSR1";
char password[] = "00000000";
// ───────────────────────────────────────────────────────────────

// ── IP del servidor (la que imprime tu Arduino en Serial) ─────
IPAddress serverIP(192, 168, 1, 105);  // <-- cambiar con IP real
int serverPort = 80;
// ───────────────────────────────────────────────────────────────

int status = WL_IDLE_STATUS;

int curX    = 90;
int curY    = 90;
int laser   = 0;
const int STEP = 10;  // grados por pulsación

void sendCommand(int x, int y, int las) {
  WiFiClient client;

  Serial.print("Conectando al servidor ");
  Serial.println(serverIP);

  if (!client.connect(serverIP, serverPort)) {
    Serial.println("Error: no se pudo conectar al servidor");
    return;
  }

  String url = "/?x=" + String(x) + "&y=" + String(y) + "&laser=" + String(las);
  client.println("GET " + url + " HTTP/1.1");
  client.println("Host: " + serverIP.toString());
  client.println("Connection: close");
  client.println();

  // Lee respuesta
  unsigned long t = millis();
  while (client.available() == 0 && millis() - t < 2000) delay(1);

  String response = "";
  while (client.available()) {
    response += client.readString();
  }
  client.stop();

  // Extrae JSON de la respuesta
  int jsonStart = response.indexOf('{');
  if (jsonStart != -1) {
    Serial.println("Respuesta: " + response.substring(jsonStart));
  } else {
    Serial.println("Sin respuesta JSON");
  }
}

void setup() {
  Serial.begin(9600);

  Serial.print("Conectando a: ");
  Serial.println(ssid);

  status = WiFi.begin(ssid, password);

  if (status != WL_CONNECTED) {
    Serial.println("Error conectando al WiFi");
    while (true);
  }

  Serial.println("WiFi conectado!");
  Serial.print("IP del cliente: ");
  Serial.println(WiFi.localIP());
  Serial.println("────────────────────────────────");
  Serial.println("Controles (escribe en Serial Monitor):");
  Serial.println("  w = Servo Y arriba");
  Serial.println("  s = Servo Y abajo");
  Serial.println("  a = Servo X izquierda");
  Serial.println("  d = Servo X derecha");
  Serial.println("  l = Toggle laser");
  Serial.println("  c = Centrar (90,90)");
  Serial.println("────────────────────────────────");

  // Posición inicial
  sendCommand(curX, curY, laser);
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    bool changed = true;

    switch (cmd) {
      case 'w': curY = min(180, curY + STEP); break;
      case 's': curY = max(0,   curY - STEP); break;
      case 'a': curX = max(0,   curX - STEP); break;
      case 'd': curX = min(180, curX + STEP); break;
      case 'l': laser = laser ? 0 : 1;        break;
      case 'c': curX = 90; curY = 90;         break;
      default:  changed = false;               break;
    }

    if (changed) {
      Serial.print("Enviando → X:"); Serial.print(curX);
      Serial.print(" Y:"); Serial.print(curY);
      Serial.print(" Laser:"); Serial.println(laser ? "ON" : "OFF");
      sendCommand(curX, curY, laser);
    }
  }
}
