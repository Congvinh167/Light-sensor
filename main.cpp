#include <Wire.h>
#include <BH1750.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "FS.h"
#include <LittleFS.h>

#define FORMAT_LITTLEFS_IF_FAILED true

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("- failed to open file for appending");
    return;
  }
  if(file.print(message)){
    Serial.println("- message appended");
  } else {
    Serial.println("- append failed");
  }
  file.close();
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return;
  }

  Serial.println("- read from file:");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

// ==== WiFi and MQTT Info ====
const char* ssid = "Tien Dat";
const char* password = "dhvk142857";
const char* mqtt_server = "192.168.1.12"; 

// ==== Global Setup ====
WiFiClient espClient;
PubSubClient client(espClient);
BH1750 lightMeter;

// ==== WiFi Setup ====
void setup_wifi() {
  delay(10);
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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// ==== MQTT Reconnect ====
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

// ==== Setup ====
void setup() {
  Serial.begin(115200);
  Wire.begin(); 
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
  Serial.println("LittleFS Mount Failed");
  return;
  }
  writeFile(LittleFS,"/light_sensor.txt","Light sensor intensity:\r\n");
}

// ==== Main Loop ====
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float lux = lightMeter.readLightLevel();
  if (lux >= 0) {
    char payload[10];
    dtostrf(lux, 4, 2, payload);
    client.publish("sensor/light", payload);
    Serial.print("Light intensity: ");
    Serial.print(payload);
    Serial.println(" lux");
    String line = String(payload) + "\r\n";
    appendFile(LittleFS,"/light_sensor.txt",line.c_str());
  } else {
    Serial.println("Error reading light level.");
  }
  delay(2000);

}
