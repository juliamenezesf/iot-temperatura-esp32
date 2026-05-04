#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
DHTesp dht;

void setup_wifi() {
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);

  int tentativas = 0;

  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
  } else {
    Serial.println("\nFalha ao conectar no WiFi");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Conectando ao MQTT...");

    String clientId = "esp32-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT conectado!");
    } else {
      Serial.print("Erro MQTT: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Iniciando sistema...");

  dht.setup(15, DHTesp::DHT22);
  delay(2000); // estabilizar sensor

  setup_wifi();

  client.setServer(mqtt_server, 1883);
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi caiu, reconectando...");
    setup_wifi();
  }

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  TempAndHumidity data = dht.getTempAndHumidity();

  if (!isnan(data.temperature)) {

    char payload[50];
    sprintf(payload, "{\"temp\": %.2f, \"hum\": %.2f}", data.temperature, data.humidity);

    client.publish("maquina1/temperatura", payload);

    Serial.print("Enviado: ");
    Serial.println(payload);

  } else {
    Serial.println("Erro ao ler o sensor!");
  }

  delay(5000);
}