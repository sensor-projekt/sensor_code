#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h> 
#include <ArduinoJson.h>
#include "DHTesp.h" //https://github.com/beegee-tokyo/DHTesp

DHTesp dht;
WiFiClient wifiClient;

const char* ssid = "";
const char* password = "";
const char* apiString = "http://146.190.1.185/postmeasurement/";
const char* host = "http://146.190.1.185";
const int httpsPort = 80;
const char* sensorAuth1 = "Basic ZDNiNzYwMzQtOTUzZS00YmNmLTgwMjEtYWJhYmQ3MDc1YmZjOitIOGkjRzg0dDk=";
const char* tempSensorId = "d3b76034-953e-4bcf-8021-ababd7075bfc";
const char* tempConnectKey = "+H8i#G84t9";
const char* sensorAuth2 = "Basic OWIxYTYwMjMtNTczZi00ODMxLWI1MzktNWRiMzNiNmE2YmU5OmFPNyYrTDM0MnM=";
const char* humSensorId = "9b1a6023-573f-4831-b539-5db33b6a6be9";
const char* humConnectKey = "aO7&+L342s";

DynamicJsonDocument postData(256);
DynamicJsonDocument responseData(256);

void setup() 
{
  
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
    postData["sensorId"] = tempSensorId;
    postData["value"] = F;

    postJson(sensorAuth1);
    
    postData["sensorId"] = humSensorId;
    postData["value"] = humidity;

    postJson(sensorAuth2);

  }

  //delay 5 minutes
  delay(1000*5*60);
}
