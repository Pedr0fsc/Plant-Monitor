#include <ESP8266WiFi.h>
#include "DHT.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Define pins
#define LED1PIN D8
#define LED2PIN D7
#define LED3PIN D6
#define DHTPIN D4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Wifi parameters
#define WLAN_SSID ""
#define WLAN_PASS ""

// Adafruit IO
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME  ""
#define AIO_KEY       ""

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Publish data to feeds on Adafruit IO
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(10);

  pinMode(LED1PIN, OUTPUT);
  pinMode(LED2PIN, OUTPUT);
  pinMode(LED3PIN, OUTPUT);

  Serial.println();
  Serial.println(F("Adafruit MQTT demo"));

  // Conectando ao WiFi
  Serial.print("Conectando ao WiFi: ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
    digitalWrite(LED1PIN, HIGH);
    digitalWrite(LED2PIN, LOW);
    digitalWrite(LED3PIN, HIGH);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // LED indica conexão OK
    digitalWrite(LED1PIN, LOW);
    digitalWrite(LED2PIN, HIGH);
    digitalWrite(LED3PIN, HIGH);
  } else {
    Serial.println("\nFalha ao conectar no WiFi.");
    // LED indica erro
    digitalWrite(LED1PIN, HIGH);
    digitalWrite(LED2PIN, HIGH);
    digitalWrite(LED3PIN, LOW);
  }
}

void loop() {
  MQTT_connect();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(t) || isnan(h)) {
    Serial.println("Falha na leitura do DHT11...");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.println("°C");
    Serial.print("Umidade: ");
    Serial.print(h);
    Serial.println("%");

    temperature.publish(t);
    humidity.publish(h);
  }

  delay(60000); // 1 minuto entre envios
}

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Conectando ao MQTT... ");

  uint8_t tentativas = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Tentando novamente em 5 segundos...");
    mqtt.disconnect();
    delay(5000);
    tentativas--;
    if (tentativas == 0) {
      Serial.println("Falha na conexão MQTT. Reiniciando ESP...");
      ESP.restart();  // Reinicia o ESP após falha nas tentativas
    }
  }

  Serial.println("Conectado ao MQTT com sucesso!");
}
