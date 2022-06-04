#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h> 
#include <ArduinoJson.h>
#include "DHTesp.h" //https://github.com/beegee-tokyo/DHTesp
#include <WiFiManager.h>

DHTesp dht;
WiFiClient wifiClient;

const char* apiString = "http://146.190.1.185/postmeasurement/";
const char* host = "http://146.190.1.185";
const int httpsPort = 80;
const char* sensorAuth1 = "Basic ZDNiNzYwMzQtOTUzZS00YmNmLTgwMjEtYWJhYmQ3MDc1YmZjOitIOGkjRzg0dDk=";
const char* sensorId1 = "d3b76034-953e-4bcf-8021-ababd7075bfc";
const char* sensorKey1 = "+H8i#G84t9";
const char* sensorAuth2 = "Basic OWIxYTYwMjMtNTczZi00ODMxLWI1MzktNWRiMzNiNmE2YmU5OmFPNyYrTDM0MnM=";
const char* sensorId2 = "9b1a6023-573f-4831-b539-5db33b6a6be9";
const char* sensorKey2 = "aO7&+L342s";

DynamicJsonDocument postData(256);
DynamicJsonDocument responseData(256);

void setup() 
{
  
  Serial.begin(115200);
  
  //setup DHT22 sensor
  dht.setup(D2, DHTesp::DHT22);
  
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // Uncomment and run it once, if you want to erase all the stored information
  wifiManager.resetSettings();
  
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect(sensorId1, sensorKey1);
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
}

void postJson(String sensorAuth) {
  String json; 
  serializeJson(postData, json);

  Serial.print("json: ");
  Serial.println(json);

  //WiFiClientSecure client;
  //client.setInsecure(); //the magic line, use with caution
  //client.connect(host, httpsPort);

  HTTPClient http;
  http.begin(wifiClient, apiString);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", sensorAuth.c_str());
  int httpCode = http.POST(json);

  Serial.print("httpCode: ");
  Serial.println(httpCode);
  
  if (httpCode > 0) {
    if (httpCode != 200 && httpCode != 201) {
      String response = http.getString();
      DeserializationError error = deserializeJson(responseData, response);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
      }
      Serial.print("Something went wrong: ");
      Serial.println(response);
      Serial.println("");
    }
    else {
      String response = http.getString();
      Serial.print("response: ");
      Serial.println(response);
      Serial.println("");
    }

  }

  http.end(); //Close connection
}

void loop() 
{
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  float F = dht.toFahrenheit(temperature);

  Serial.print("WiFi.status: ");
  Serial.println(WiFi.status());

  Serial.print("temperature: " );
  Serial.println(F);

  Serial.print("humidity: ");
  Serial.println(humidity);
  
  if (WiFi.status() == WL_CONNECTED) 
  { 
    postData["sensorId"] = sensorId1;
    postData["value"] = F;

    postJson(sensorAuth1);
    
    postData["sensorId"] = sensorId2;
    postData["value"] = humidity;

    postJson(sensorAuth2);

  }

  //delay 5 minutes
  delay(1000*5*60);
}
