// UTBM Innovation CRUNCH Lab 
//
// MQTT usage example based on the Joël Gähwiler MQTT lib (https://github.com/256dpi/arduino-mqtt) 
// with addition of a Serial Menu to configure the board with net informations and to test pub/sub.
//
// by Olivier LAMOTTE

#include <Arduino.h>
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif
#include <MQTT.h>
#include <EEPROM.h>


// Serial input managment
String inputString = "";
boolean stringComplete = false;

// Current SSID and PWD
String ssid = "";
String pwd = "";

// Futur SSID and PWD
String newSSID = "";
String newPWD = "";

// Current Broker Host
String broker = "";

// Futur Broker host
String newBroker = "";

// Current ID
String id = "";

// Futur ID
String newId = "";

// Publish topic
String newPublishTopic = "";

// Publish message
String newMessage = "";

// Subscribe topic
String newSubscribeTopic = "";

// Menu managment
const int MENU_OFF = -1;
const int MAIN_MENU_LEVEL = 0;
const int SSID_MENU_LEVEL = 1;
const int PWD_MENU_LEVEL = 2;
const int BROKER_MENU_LEVEL = 3;
const int ID_MENU_LEVEL = 4;
const int PUBLISH_MENU_LEVEL = 5;
const int SUBSCRIBE_MENU_LEVEL = 6;
const int MESSAGE_MENU_LEVEL = 7;
const int REBOOT_MENU_LEVEL = 10;

int menuLevel = 0;

const int EEPROM_NET_SSID_SIZE = 64;
const int EEPROM_NET_PWD_SIZE = 64;
const int EEPROM_BROKER_HOST_SIZE = 16;
const int EEPROM_ID_SIZE = 32;

const int EEPROM_NET_SSID_OFFSET = 0;
const int EEPROM_NET_PWD_OFFSET = EEPROM_NET_SSID_SIZE;
const int EEPROM_BROKER_HOST_OFFSET = EEPROM_NET_PWD_OFFSET + EEPROM_NET_PWD_SIZE;
const int EEPROM_ID_OFFSET = EEPROM_BROKER_HOST_OFFSET + EEPROM_BROKER_HOST_SIZE;

// Configuration information
bool configured = false;

const int WIFI_CONNECTION_TRY_COUNT_MAX = 20;
const int BROKER_CONNECTION_TRY_COUNT_MAX = 20;

WiFiClient net;
MQTTClient client;

int wifiStatus = WL_IDLE_STATUS;

unsigned long lastMillis = 0;

unsigned int aliveCounter = 0;


void eepromWrite(int address, String data, unsigned int maxSize, boolean commit) {
  for (unsigned int idx = 0; idx < data.length(); idx++) {
    EEPROM.write(0x0F + address + idx, data.charAt(idx));
  }
  if (commit) EEPROM.commit();
}

String eepromRead(int address, unsigned int maxSize) {
  String result = "";
  for (unsigned int idx = 0x0F + address; idx < (0x0F + address + maxSize); idx++) {

    int i =  EEPROM.read(idx);
    if (i == 255) break;
    result += (char)i;
  }
  return result;
}


// Reboot the board
void rebootAsked() {
  if (!newSSID.equals("") && !newPWD.equals("") && !newBroker.equals("") && !newId.equals("")) {
    for (unsigned int i = 0 ; i < EEPROM.length() ; i++) {
      EEPROM.write(i, 255);
    }

    eepromWrite(EEPROM_NET_SSID_OFFSET, newSSID, EEPROM_NET_SSID_SIZE, false);
    eepromWrite(EEPROM_NET_PWD_OFFSET, newPWD, EEPROM_NET_PWD_SIZE, false);
    eepromWrite(EEPROM_BROKER_HOST_OFFSET, newBroker, EEPROM_BROKER_HOST_SIZE, false);
    eepromWrite(EEPROM_ID_OFFSET, newId, EEPROM_ID_SIZE, true);
  }
  delay(150);

  ESP.restart();
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

int connectWifi(int connectionTryCountMax) {

  int connectionTryCount = 0;

  char ssidArray[ssid.length()];
  ssid.toCharArray(ssidArray, ssid.length()+1);
  
  char pwdArray[pwd.length()];
  pwd.toCharArray(pwdArray, pwd.length()+1);
  
  Serial.print("Attempting to connect to Network named : ");
  Serial.println(ssidArray);

  WiFi.begin(ssidArray, pwdArray);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED && connectionTryCount < connectionTryCountMax) {
    delay(500);
    Serial.print(".");
    connectionTryCount++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    printWifiStatus();
    return WL_CONNECTED;
  }

  return WL_CONNECT_FAILED;
}


void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  if (topic.equals(newSubscribeTopic)) {
    client.unsubscribe(newSubscribeTopic);
  }

  // DO SOMETHING WITH RECEIVED MESSAGES
}

// Try to connect the board to network
bool connect(int connectionTryCountMax) {

  Serial.println("checking wifi...");
  int wifiStatus = connectWifi(WIFI_CONNECTION_TRY_COUNT_MAX);
  if (wifiStatus != WL_CONNECTED) {
    Serial.println("Wifi connection failed...");
    return false;
  }

  Serial.print("\nconnecting broker...");


  char brokerArray[broker.length()];
  broker.toCharArray(brokerArray, broker.length()+1);
    
  client.begin(brokerArray, net);
  client.onMessage(messageReceived);

  char idArray[id.length()];
  id.toCharArray(idArray, id.length()+1);

  int connectionTryCount = 0;
  while (connectionTryCount < connectionTryCountMax && !client.connect(idArray, "try", "try")) {
    Serial.print(".");
    connectionTryCount++;
    delay(1000);
  }

  if (connectionTryCount == connectionTryCountMax) {
    return false;
  }

  Serial.println("\nconnected!");

  // SUBSCRIBE TOPIC HERE
  // ...
  // client.subscribe("/.../...")
  // -------

  return true;
}

void readSerial() {

  while (Serial.available()) {
    char inChar = (char)Serial.read();;
    if (inChar != '\n' && inChar !='\r') {
      inputString += inChar;
    }
    if (inChar == '\n') {
      stringComplete = true;
    }
  }

}

String toString(uint8_t * payload, int length) {
  String result = "";
  for (int idx = 0; idx < length; idx++) {
    result += (char)(*(payload+idx));
  }
  return result;
}

void displayHelp() {

  Serial.println("Commands");
  Serial.println();
  Serial.println("1 - Define the network connection information (SSID and PWD)");
  Serial.println("2 - Define the Broker host IP or URL");
  Serial.println("3 - Define object ID");
  Serial.println("4 - Read all parameters (except PWD)");
  Serial.println("5 - Publish a value on a est topic");
  Serial.println("6 - Subscribe to a topic for test");
  Serial.println("0 - Reboot with new parameters");
  Serial.println();
  Serial.println("Enter your choice");
  Serial.println();
}

void displayCurrentConfig(bool afterReboot) {

  Serial.print("SSID : ");
  Serial.print(ssid);
  if (afterReboot) {
    Serial.print(" => ");
    Serial.println(newSSID);
  } else {
    Serial.println("");
  }

  Serial.print("BROKER HOST : ");
  Serial.print(broker);
  if (afterReboot) {
    Serial.print(" => ");
    Serial.println(newBroker);
  } else {
    Serial.println("");
  }

  Serial.print("MQTT ID : ");
  Serial.print(id);
  if (afterReboot) {
    Serial.print(" => ");
    Serial.println(newId);
  } else {
    Serial.println("");
  }
}

void setup() {

  delay(500);
  Serial.begin(9600);

  EEPROM.begin(512);
  delay(150);
  
  ssid = eepromRead(EEPROM_NET_SSID_OFFSET, EEPROM_NET_SSID_SIZE);
  pwd = eepromRead(EEPROM_NET_PWD_OFFSET, EEPROM_NET_PWD_SIZE);
  broker =  eepromRead(EEPROM_BROKER_HOST_OFFSET, EEPROM_BROKER_HOST_SIZE);
  id = eepromRead(EEPROM_ID_OFFSET, EEPROM_ID_SIZE);

  newSSID = ssid;
  newPWD = pwd;
  newBroker = broker;
  newId = id;

  delay(3000);

  Serial.println("");
  Serial.println("");
  Serial.println("---------------------------------------");
  Serial.println("-- Innovation CRUNCH Lab development --");
  Serial.println("---------------------------------------");

  Serial.println("");
  Serial.println("");


  // DECLARE YOUR YOUR BOARD PIN CONFIGURATION HERE
  // ...
  // pinMode(inputPin, INPUT);
  // -----

  if (ssid != "" && pwd != "" && broker != "" && id != "") {
    Serial.println("Configuration found");

    displayCurrentConfig(false);
    
    configured = connect(BROKER_CONNECTION_TRY_COUNT_MAX);
    menuLevel = MENU_OFF;

    if (!configured) {
      Serial.println("Wrong configuration !");
      Serial.println("");
      stringComplete = true;
      menuLevel = MAIN_MENU_LEVEL;
    } else {
      Serial.println("Condifugation done !");
      Serial.println("");
    }

  } else {
    if (ssid == "") {
      Serial.println("SSID must be set");
    } else {
      Serial.print("SSID : ");
      Serial.println(ssid);
    }
    if (pwd == "") {
      Serial.println("Password must be set");
    } else {
      Serial.println("A password is set");
    }
    if (broker == "") {
      Serial.println("Broker host must be set");
    } else {
      Serial.print("BROKER HOST : ");
      Serial.println(broker);
    }
    if (id == "") {
      Serial.println("MQTT Object identifier must be set");
    } else {
      Serial.print("MQTT OBJECT ID : ");
      Serial.println(id);
    }

    Serial.println("Configuration must be completed !");
    Serial.println("");
    configured = false;
    menuLevel = MENU_OFF;
    stringComplete = true;
  }
}

void loop() {

  if (Serial.available() > 0) {
    readSerial();
  }

  if (stringComplete) {

    if (menuLevel == MENU_OFF) {

      displayHelp();
      menuLevel = MAIN_MENU_LEVEL;
      stringComplete = false;
      inputString = "";

    } else {

      switch (menuLevel) {
        case MAIN_MENU_LEVEL:
        if (inputString.startsWith("1")) {
          Serial.println("==> Network connection definition");
          Serial.println("Enter the new SSID : ");
          menuLevel = SSID_MENU_LEVEL;
        } else if (inputString.startsWith("2")) {
          Serial.println("==> Broker definition");
          Serial.println("Enter the Broker host : ");
          menuLevel = BROKER_MENU_LEVEL;
        } else if (inputString.startsWith("3")) {
          Serial.println("==> MQTT ID definition");
          Serial.println("Enter the MQTT ID for this object : ");
          menuLevel = ID_MENU_LEVEL;
        } else if (inputString.startsWith("4")) {
          displayCurrentConfig(true);
          menuLevel = MAIN_MENU_LEVEL;
        } else if (inputString.startsWith("5")) {
          Serial.println("==> PUB Topic definition");
          Serial.println("Enter a topic : ");
          menuLevel = PUBLISH_MENU_LEVEL;
        } else if (inputString.startsWith("6")) {
          Serial.println("==> SUB Topic definition");
          Serial.println("Enter a topic : ");
          menuLevel = SUBSCRIBE_MENU_LEVEL;
        } else if (inputString.startsWith("0")) {
          Serial.println("==> Reebot [Y|N] ?");
          menuLevel = REBOOT_MENU_LEVEL;
        } else {
          if (inputString.equals("")) {
            menuLevel = MAIN_MENU_LEVEL;
            displayHelp();
          } else {
            Serial.println("Enter a valid command...");
          }
        }
        break;
        case SSID_MENU_LEVEL:
          newSSID = inputString;
          Serial.println("Enter the new PWD : ");
          menuLevel = PWD_MENU_LEVEL;
        break;
        case PWD_MENU_LEVEL:
          newPWD = inputString;
          Serial.println("Network configuration done !");
          Serial.println("Reboot is needed");
          displayHelp();
          menuLevel = MAIN_MENU_LEVEL;
        break;
        case BROKER_MENU_LEVEL:
          newBroker = inputString;
          Serial.println("Broker configuration done !");
          Serial.println("Reboot is needed");
          displayHelp();
          menuLevel = MAIN_MENU_LEVEL;
        break;
        case ID_MENU_LEVEL:
          newId = inputString;
          Serial.println("Broker configuration done !");
          Serial.println("Reboot is needed");
          displayHelp();
          menuLevel = MAIN_MENU_LEVEL;
        break;
        case PUBLISH_MENU_LEVEL:
          newPublishTopic = inputString;
          Serial.println("Enter message : ");
          menuLevel = MESSAGE_MENU_LEVEL;
        break;
        case MESSAGE_MENU_LEVEL:
          newMessage = inputString;
          Serial.println("Publish done");
          menuLevel = MAIN_MENU_LEVEL;
        break;
        case SUBSCRIBE_MENU_LEVEL:
          newSubscribeTopic = inputString;
          if (newSubscribeTopic != "") {
            Serial.print("Subscribe done for topic : ");
            Serial.println(newSubscribeTopic);
            client.subscribe(newSubscribeTopic);
          }
          menuLevel = MAIN_MENU_LEVEL;
        break;
        case REBOOT_MENU_LEVEL:
          if (inputString.startsWith("Y") || inputString.startsWith("y")) {
            Serial.println("Reboot confirmed...");
            delay(1000);
            for (int i = 0; i < 10; i++) {
              Serial.print(".");
              delay(1000 - (i * 100));
            }
            rebootAsked();
          } else {
            Serial.println("Reboot aborded !");
            delay(3000);
            menuLevel = MENU_OFF;
          }
        break;
        default:
          Serial.println("Default");
          Serial.println(inputString);
        break;
      }

      inputString = "";
      stringComplete = false;

    }

  }

  if (configured) {
    client.loop();
    delay(10);  // <- fixes some issues with WiFi stability


    if (!client.connected()) {
      connect(BROKER_CONNECTION_TRY_COUNT_MAX);
    }

    if (client.connected()) {

      if (newPublishTopic != "" && newMessage != "") {
        client.publish(newPublishTopic, newMessage);
        newPublishTopic = "";
        newMessage = "";
      }

      // PUBLISH MESSAGE HERE
      // ...
      // -----

      // publish a message roughly every minute.
      if (millis() - lastMillis > 60000) {
        lastMillis = millis();

        String msg = id + " is alive since " + aliveCounter + " minute" + aliveCounter == 0 ? "" : "s";

        String aliveTopic = "/ALIVE/";
        aliveTopic.concat(id);

        client.publish(aliveTopic, msg);

        aliveCounter++;
      }
    }
  }
}
