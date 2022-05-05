#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "DHTesp.h" //https://github.com/beegee-tokyo/DHTesp

DHTesp dht;
WiFiClient wifiClient;

const char* ssid = "";
const char* password = "";
const char* host = "10.0.0.48";
const char* tempSensorId = "";
const char* tempConnectKey = "";
const char* humSensorId = "";
const char* humConnectKey = "";
String apiString;
String tempApiString = "";
String humApiString = "";
 
DynamicJsonDocument postData(256);
DynamicJsonDocument responseData(256);

void postJson() {
  String json; 
  serializeJson(postData, json);

  HTTPClient http; //Object of class HTTPClient
  
  http.begin(wifiClient, apiString);

  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(json);

  if (httpCode > 0) {
    if (httpCode != 200) {
      String response = http.getString();
      DeserializationError error = deserializeJson(responseData, response);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
      }
      const char* message = responseData["message"];
      Serial.print("Something went wrong: ");
      Serial.println(message);
      Serial.println("");
    }
  }

  http.end(); //Close connection
}

void setup() 
{
  String cpString = ""; 
  cpString += "http://";
  cpString += host;
  cpString += ":8080/api/public/measurement/";
  
  tempApiString += cpString;
  tempApiString += tempSensorId;
  
  humApiString += cpString;
  humApiString += humSensorId;
  
  Serial.begin(115200);
  
  //setup DHT22 sensor
  dht.setup(D2, DHTesp::DHT22);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Connecting...");
  }
  
  // Print the IP address
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println();
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
    postData["value"] = humidity;
    postData["connectKey"] = humConnectKey;

    apiString = "";
    apiString += humApiString;
    postJson();
    
    //Begin second post
    postData["value"] = F;
    postData["connectKey"] = tempConnectKey;

    apiString = "";
    apiString += tempApiString;
    
    postJson();
  }

  //delay 10 minutes
  delay(1000*30);
}