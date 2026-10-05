#include <SPI.h>
#include <Ethernet.h>

// MAC
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// IP en tu red (IMPORTANTE)
IPAddress ip(192, 168, 1, 10);

// Servidor en puerto 80
EthernetServer server(80);

// Sensor en A0
int sensorPin = A0;

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  Serial.println("Servidor Web Arduino con Sensor de Luz");

  Ethernet.begin(mac, ip);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("No se detecta el shield Ethernet");
    while (true) { delay(1); }
  }

  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Cable Ethernet no conectado");
  }

  server.begin();

  Serial.print("Servidor en: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  EthernetClient client = server.available();

  if (client) {
    Serial.println("Cliente conectado");

    boolean currentLineIsBlank = true;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        if (c == '\n' && currentLineIsBlank) {

          // Leer sensor SOLO A0
          int valorLuz = analogRead(sensorPin);

          // Respuesta HTTP
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 3"); // se actualiza cada 3 segundos
          client.println();

          // Página web
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head>");
          client.println("<title>Sensor de Luz</title>");
          client.println("</head>");
          client.println("<body style='font-family:Arial;'>");

          client.println("<h1>Sensor de Luminosidad</h1>");
          client.print("<h2>Valor: ");
          client.print(valorLuz);
          client.println("</h2>");

          // Interpretación del valor
          if (valorLuz < 300) {
            client.println("<p style='color:red;'><b>Ambiente oscuro</b></p>");
          } else {
            client.println("<p style='color:green;'><b>Ambiente iluminado</b></p>");
          }

          client.println("</body>");
          client.println("</html>");

          break;
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }

    delay(1);
    client.stop();
    Serial.println("Cliente desconectado");
  }
}