class AirQuality {

private:
  bool _isOk;

public:
  float pm25;
  float pm10;
  float pm25normalized = 0.0;
  float pm10normalized = 0.0;

  float humidity;
  float temperature;
  float altitude;
  float pressure;

  AirQuality() {
    this->_isOk = false;
  }

  AirQuality(float pm25, float pm10, float temperature, float humidity, float altitude, float pressure) {
    this->pm10 = pm10;
    this->pm25 = pm25;
    this->temperature = temperature;
    this->humidity = humidity;
    this->altitude = altitude;
    this->pressure = pressure / 100.0F;
    this->_isOk = true;
  }

  float normalizePM25() {
    if (this->pm25normalized == 0.0) {
      if (this->humidity > 0) {
        this->pm25normalized = this->pm25 / (1.0 + 0.48756 * pow((this->humidity / 100.0), 8.60068));
      } else {
        this->pm25normalized = pm25;
      }
    }
    return this->pm25normalized;
  }

  float normalizePM10() {
    if (this->pm10normalized == 0.0) {
      if (this->humidity > 0) {
        this->pm10normalized = this->pm10 / (1.0 + 0.81559 * pow((this->humidity / 100.0), 5.83411));
      } else {
        this->pm10normalized = this->pm10;
      }
    }
    return this->pm10normalized;
  }

  String toString() {
    return "PM10: " + String(this->pm10) + ", PM2.5: " + String(this->pm25) + ", Temperature: " + String(this->temperature) + ", Humidity: " + String(this->humidity) + ", Pressure: " + String(this->pressure) + ", Altitude: " + String(this->altitude);
  }

  String toShortString() {
    return "temp: " + String((int)this->temperature) + " / hum: " + String((int)this->humidity) + " / pre: " + String((int)this->pressure) + " / alt: " + String((int)this->altitude);
  }

  bool isOk() {
    return this->_isOk;
  }
};