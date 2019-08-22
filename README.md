# SimpleESPMQTT
A simple Arduino project that allow pub/sub with MQTT from ESP32 and ESP8266 board (tested with Adafruit ESP32 Feather HUZZAH and Adafruit ESP8266 Feather HUZZAH).

[This project is an example of using the Joël Gähwiler MQTT lib.](https://github.com/256dpi/arduino-mqtt "Joël Gähwiler's github page")

## Platformio configuration file
This project use as IDE the Visual Studio Code application with the PlatformIO plugin.
The configuration file platformio.ini is done for compile and deploy the program for the ESP32 and the ESP8266 in the same time. So you have to comment one of the two *"env:"* part of the configuration if you use just one board.

Adapt the *"upload_port"* parameter for each board profile according to your configuration.

## Source code adaptation
To use this example, you have to fill some data according to your configuration :
1. Information for network connection
* The SSID (line 16)
* The password (line 17)
2. The MQTT Broker information
* The Broker host (line 21)
* The Broker port (line 22)
3. The Object ID (Optional)
* The ESP8266 Board ID (line 10)
* The ESP32 Board ID (line 13)

