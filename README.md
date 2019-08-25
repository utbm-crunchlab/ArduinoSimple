# SimpleESPMQTT
A simple Arduino project that allow pub/sub with MQTT from ESP32 and ESP8266 board (tested with Adafruit ESP32 Feather HUZZAH, Adafruit ESP8266 Feather HUZZAH and ESP-01).

[This project is an example of using the Joël Gähwiler MQTT lib.](https://github.com/256dpi/arduino-mqtt "Joël Gähwiler's github page")

## Platformio configuration file
This project use as IDE the Visual Studio Code application with the PlatformIO plugin.
The configuration file platformio.ini is done for compile and deploy the program for the ESP32, the ESP8266 and the ESP_01 boards in the same time. So you have to comment the *"env:"* part of the configuration that you don't need.

Adapt the *"upload_port"* parameter for each board profile according to your configuration.

## Usage
When the board is started, it try to find a previous configuration stored in it's EEPROM.
If a valid configuration is found (SSID, PWD, Broker Host and board Id), it try to connect.
In the other hand, a menu is displayed to configure the board.

When you press a key the menu is displayed again.

Two menu options allow testing the MQTT connection by publish or subscribe to a topic.

## Source code adaptation
See comments in the source code to add your parts.

