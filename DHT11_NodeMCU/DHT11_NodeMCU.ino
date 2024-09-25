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
#define col 16
#define lin 2
#define addr 0x3F

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(addr, col, lin);

// Wifi parameters
#define WLAN_SSID ""
#define WLAN_PASS ""

// Adafruit IO
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME ""
#define AIO_KEY ""
WiFiClient client;

// Setup MQTT client
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Publish data to feeds on Adafruit IO
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");

void MQTT_connect();

void setup() {
  Serial.begin(9600);
  dht.begin();
  delay(10);

  lcd.begin(col, lin);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Adafruit MQTT demo");

  pinMode(D8, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D6, OUTPUT);

  Serial.println();
  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  if (WiFi.status() != WL_CONNECTED) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      digitalWrite(D8, true);
      digitalWrite(D7, false);
      digitalWrite(D6, true);
    }
    if (WiFi.status() != WL_DISCONNECTED) {
      digitalWrite(D8, true);
      digitalWrite(D7, true);
      digitalWrite(D6, false);
    }
  }
  digitalWrite(D8, false);
  digitalWrite(D7, true);
  digitalWrite(D6, true);
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

}

void loop() {
  MQTT_connect();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(t) || isnan(h)) 
  {
  Serial.println("Falha na leitura do dht11...");
  } 
  else 
  {
  //publish temperature and humidity
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println("°C");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");
  
  temperature.publish(t);
  humidity.publish(h);
    
  delay(60000);
  }
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);
       retries--;
       if (retries == 0) {
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
