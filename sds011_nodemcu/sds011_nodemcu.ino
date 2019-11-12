#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SDS011.h>
#include <SoftwareSerial.h>

#define SDS_RX D1
#define SDS_TX D2
#define SAMPLES 5

const int sleep_duration = 10 * 60000;        //sleep duration 10 min

const char* ssid     = "";          //wifi ssid goes here
const char* password = "";          //wifi password goes here

const char* host = "api.thingspeak.com";
const char* iftttHost = "maker.ifttt.com";

String apiKey = "";                 // thingspeak.com api key goes here
String iftttApiKey = "";			// IFTTT webhooks api key
String iftttEvent = "";				// IFTTT notification event

float p10,p25;
int error;

struct Air {
	float pm25;
	float pm10;
	float humidity;
	float temperature;
};

ESP8266WebServer server(80);
WiFiClient client;
SDS011 sds;

void setup() {
	Serial.begin(9600);
	sds.begin(SDS_RX, SDS_TX);
	connectToWiFi();
}

void loop() {
	Air airData = readPolution();
	if (airData.pm25 > 0.0) {
		sendThings(airData);

		if (airData.pm25 > 25.0 || airData.pm10 > 50.0) {
			sendIFTTT(airData);
		}
	}

	sds.sleep();

	server.handleClient();

	delay(sleep_duration);

	sds.wakeup();

	delay(5000);
}

void sendThings(Air airData) {
	if (!client.connect(host,80)) {
		return;
	}

	String postStr = apiKey;
	postStr +="&field1=";
	postStr += String(airData.pm25);
	postStr +="&field2=";
	postStr += String(airData.pm10);
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

	client.stop();
}

void sendIFTTT(Air airData) {
	if (!client.connect(iftttHost, 80)) {
		return;
	}

	String postStr = "{\"value1\":\"" + String(airData.pm25) + "\",\"value2\":\"" + String(airData.pm10) + "\"}";

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
	server.send(200, "text/plain", "Nothing to see here, move along.");
}

Air readPolution(){
	error = sds.read(&p25, &p10);
	if (!error) {
		Air result = (Air){calculatePolutionPM25(p25), calculatePolutionPM10(p10), 0.0, 0.0};
		return result;
	} else {
		Serial.println("Error reading SDS011");
		Serial.println(error);
		return (Air){0.0, 0.0, 0.0, 0.0};
	}
}

//Correction algorythm thanks to help of Zbyszek Kiliański (Krakow Zdroj)
float normalizePM25(float pm25, float humidity){
	return pm25 / (1.0 + 0.48756 * pow((humidity / 100.0), 8.60068));
}

float normalizePM10(float pm10, float humidity){
	return pm10 / (1.0 + 0.81559 * pow((humidity / 100.0), 5.83411));
}

float calculatePolutionPM25(float pm25){
	return pm25;
}

float calculatePolutionPM10(float pm10){
	return pm10;
}
