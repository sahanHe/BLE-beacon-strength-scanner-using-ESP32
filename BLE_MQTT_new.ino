/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/
//#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

const char* ssid     = "Nokia 8";
const char* password = "12345678a";
const char* mqtt_server = "mqtt.eclipse.org";


String info="{\"id\" : 8 ";// json message chr sequence

WiFiClient espClient;
PubSubClient client(espClient);

int scanTime = 5; //In seconds
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      //Serial.printf("Advertised Device: %s", advertisedDevice.getName());
      //Serial.printf("strength of device %s is: %d \n", advertisedDevice.getAddress().toString().c_str(), advertisedDevice.getRSSI());
      //String addesss=(String)advertisedDevice.getRSSI();
      
      //String info = "strength of device " + (String)advertisedDevice.getAddress().toString().c_str()+ " is" + (String)advertisedDevice.getRSSI();
      info+=",\""+(String)advertisedDevice.getAddress().toString().c_str()+ "\":" + String(advertisedDevice.getRSSI()).c_str();
      //String x = (String)*advertisedDevice.getAddress().toString().c_str();
      
    }
};


void reconnect() { //if MQTT connection disconnected reconnect using this func
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("EN3250/ESP32");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883); 

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  info+="}";
  //info="{\"id\" :1,\"UoM_Wireless1\"1:-56,\"UoM_Wireless2\":-97}";
  Serial.println(info);
  char msgCharArray[5000];
  info.toCharArray(msgCharArray,5000);
  client.publish("EN3250/ESP32",info.c_str());
  
  info="{\"id\" : 8 ";
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());


//  Serial.println(info);
  
  Serial.println("Scan Done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(2000);
  
//  client.publish("group1ENTC","published");
}
