#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SDS011.h>
#include <SoftwareSerial.h>

#define SDS_RX D1
#define SDS_TX D2
#define SAMPLES 5

//#define DHTPIN D4 
//#define DHTTYPE DHT22   

const int sleep_duration = 10 * 60000;        //sleep duration 10 min

const char* ssid     = "";          //wifi ssid goes here
const char* password = "";          //wifi password goes here
const char* host = "api.thingspeak.com";
String apiKey = "";                 // thingspeak.com api key goes here

float p10,p25;
int error;

struct Air {
  float pm25;
  float pm10;
  float humidity;
  float temperature;
};


//DHT_Unified dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);
WiFiClient client;
SDS011 sds;

void setup() {
  Serial.begin(9600);
  sds.begin(SDS_RX,SDS_TX);
  //dht.begin();
  connectToWiFi();
}

void loop() {
  Air airData = readPolution();
  if (client.connect(host,80) & airData.pm25 > 0.0) {
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(airData.pm25);
    postStr +="&field2=";
    postStr += String(airData.pm10);
//    postStr +="&field3=";
//    postStr += String(airData.humidity);
//    postStr +="&field4=";
//    postStr += String(airData.temperature);
    postStr += "\r\n\r\n";
  
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();

  sds.sleep();
  delay(sleep_duration);
  
  server.handleClient();

  sds.wakeup();
  delay(5000);
}

void connectToWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  Serial.print("Connecting to ");
  Serial.println(ssid); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  startServer();
}

void startServer(){
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void handleRoot() {
    Air airData = readPolution();
    server.send(200, "text/plain", "PM2.5: " + String(airData.pm25) + " (" + String(calculatePolutionPM25(airData.pm25)) + "% normy) | PM10: " +  String(airData.pm10) + " (" + String(calculatePolutionPM10(airData.pm10)) + "% normy) | Temperature: " + airData.temperature + " | Humidity: " + airData.humidity);  
}

Air readPolution(){
//  float temperature, humidity;
  error = sds.read(&p25,&p10);
  if (!error) {
//    sensors_event_t event;  
//    dht.temperature().getEvent(&event);
//    if (isnan(event.temperature)) {
//      Serial.println("Error reading temperature!");
//    } else {
//      temperature = event.temperature;
//    }
//  
//    dht.humidity().getEvent(&event);
//    if (isnan(event.relative_humidity)) {
//      Serial.println("Error reading humidity!");
//    } else {
//      humidity = event.relative_humidity;
//    }
//   
    Air result = (Air){calculatePolutionPM25(p25), calculatePolutionPM10(p10), 0, 0};
    return result;
  } else {
    Serial.println("Error reading SDS011");
    Serial.println(error);
    return (Air){0.0, 0.0, 0.0, 0.0};
  }
}

//Correction algorythm thanks to help of Zbyszek Kiliański (Krakow Zdroj)
float normalizePM25(float pm25, float humidity){
  return pm25/(1.0+0.48756*pow((humidity/100.0), 8.60068));
}

float normalizePM10(float pm10, float humidity){
  return pm10/(1.0+0.81559*pow((humidity/100.0), 5.83411));
}

float calculatePolutionPM25(float pm25){
  return pm25;
}

float calculatePolutionPM10(float pm10){
  return pm10;
}
