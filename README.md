# esp8266-sds011
SDS011 particulate matter sensor on NodeMCU board (ESP8266)

- Support for Nova Fitness SDS011 particulate matter sensor family
- Support for BME280 humidity, pressure, temperature, and altitude sensor
- SSD1306 display as a presentation layer
- Sends data to ThingsSpeak
- IFTTT Notifications added

Previous/simpler version available at https://gist.github.com/piotrkpaul/f85efa61968cd5115dddb78bc4be15fc

# To set up IFTTT notifications

1. Log in / create account at IFTTT.com
2. Go to your profile, create
3. IF webhooks > set event name
4. THEN notifications > select notification type
5. Set your message, value1 is PM2.5, value2 is PM10, create message
6. Find webhooks in your profile, click documentation
7. Copy your api key.
