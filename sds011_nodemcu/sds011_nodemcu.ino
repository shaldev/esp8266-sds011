#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <SdsDustSensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AirQuality.h"
#include "Config.h"

#define SSD_SCL D1
#define SSD_SDA D2
#define BME_SDA D6
#define BME_SCL D7
#define SDS_RX_PIN D3
#define SDS_TX_PIN D4
#define SEALEVELPRESSURE_HPA (1013.25)
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

const int sleep_duration = 10 * 60000;  //sleep duration 10 min
const char* thingspeakHost = "api.thingspeak.com";
const char* iftttHost = "maker.ifttt.com";

Adafruit_BME280 bme;
WiFiClient client;
SdsDustSensor sds(SDS_RX_PIN, SDS_TX_PIN);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(9600);
  delay(100);

  if (apiKey == "") {
    Serial.println("Missing Thingspeak API key");
  }

  if (iftttApiKey == "" || iftttEvent == "") {
    Serial.println("Missing IFTTT API key or Event");
  }

  sds.begin();
  sds.setQueryReportingMode();

  Wire.begin(BME_SDA, BME_SCL);
  if (!bme.begin(0x76, &Wire)) {
    Serial.println("BME sensor failed to start");
  }

  Wire.begin(SSD_SDA, SSD_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
  }

  drawOnDisplay("Connecting to Wifi [" + String(ssid) + "]");
  connectToWiFi();
}

void loop() {
  sds.wakeup();
  delay(5000);

  AirQuality airData = readPolution();

  if (airData.isOk()) {
    Serial.println(airData.toString());

    if (airData.pm25 > 0.0 || airData.pm10 > 0.0) {
      //Serial.println("DEBUG: Sending to thingsSpeak");
      sendThings(airData);

      if (airData.pm25 > 40.0 || airData.pm10 > 60.0) {
        sendIFTTT(airData);
      }
    }

    drawOnDisplay(airData);
  }

  WorkingStateResult state = sds.sleep();

  if (state.isWorking()) {
    Serial.println("Problem with sleeping the sensor.");
  } else {
    Serial.println("Sensor is sleeping");
    delay(sleep_duration);
  }
}

void sendThings(AirQuality data) {
  if (apiKey == "") {
    return;
  }

  if (!client.connect(thingspeakHost, 80)) {
    Serial.println("Could not connect to Thingspeak");
    return;
  }

  String postStr = apiKey;
  postStr += "&field1=" + String(data.normalizePM25());
  postStr += "&field2=" + String(data.normalizePM10());
  postStr += "&field5=" + String(data.humidity);
  postStr += "&field6=" + String(data.temperature);
  postStr += "&field7=" + String(data.pressure);
  postStr += "\r\n\r\n";

  client.print("POST /update HTTP/1.1\n");
  client.print("Host: api.thingspeak.com\n");
  client.print("Connection: close\n");
  client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
  client.print("Content-Type: application/x-www-form-urlencoded\n");
  client.print("Content-Length: ");
  client.print(postStr.length());
  client.print("\n\n");
  client.print(postStr);

  client.stop();
  //Serial.println("DEBUG: Data sent to thinghSpeak");
}

void sendIFTTT(AirQuality airData) {
  if (iftttApiKey == "" || iftttEvent == "") {
    return;
  }

  if (!client.connect(iftttHost, 80)) {
    Serial.println("Could not connect to IFTTT");
    return;
  }

  String postStr = "{\"value1\":\"" + String(airData.normalizePM25()) + "\",\"value2\":\"" + String(airData.normalizePM10()) + "\"}";

  client.print("POST /trigger/" + iftttEvent + "/with/key/" + iftttApiKey + " HTTP/1.1\n");
  client.print("Host: maker.ifttt.com\n");
  client.print("Connection: close\n");
  client.print("Content-Type: application/json\n");
  client.print("Content-Length: ");
  client.print(postStr.length());
  client.print("\n\n");
  client.print(postStr);

  client.stop();
}

void connectToWiFi() {
  if (ssid == "" || password == "" || apiKey == "") {
    Serial.println("Wifi connection skipped");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("Connecting to " + String(ssid));
  long unsigned start = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    if (millis() - start >= 30000) {
      Serial.println("Could not connect to " + String(ssid));
      return;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  drawOnDisplay("Connected to Wifi");
}

AirQuality readPolution() {
  drawOnDisplay("Reading sensors...");
  Wire.begin(BME_SDA, BME_SCL);

  PmResult pm = sds.queryPm();

  if (pm.isOk()) {
    AirQuality result = AirQuality(pm.pm25,
                                   pm.pm10,
                                   bme.readTemperature(),
                                   bme.readHumidity(),
                                   bme.readAltitude(SEALEVELPRESSURE_HPA),
                                   bme.readPressure());

    return result;
  }

  Serial.println("Error reading SDS011: " + pm.statusToString());
  return AirQuality();
}

void drawOnDisplay(AirQuality data) {
  Wire.begin(SSD_SDA, SSD_SCL);
  
  display.clearDisplay();
  display.display();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("PM10: " + String((int)data.normalizePM10()));
  display.println("PM25: " + String((int)data.normalizePM25()));
  display.setTextSize(1);
  display.println(data.toShortString());
  display.display();
}

void drawOnDisplay(String msg) {
  Wire.begin(SSD_SDA, SSD_SCL);

  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(msg);
  display.display();
}