#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h> 
#include <ArduinoJson.h>
#include <FS.h>
#include "DHTesp.h" //https://github.com/beegee-tokyo/DHTesp

DHTesp dht;

const char* ssid = "";
const char* password = "";
const char* hostURL = "https://thesensorproject.org/api/public/measurement/";
const char* host = "https://thesensorproject.org";
const int httpsPort = 443;
const char* sensorId = "";
const char* connectKey = "";
String apiString = "";
 
DynamicJsonDocument postData(256);
DynamicJsonDocument responseData(256);

void setup() 
{
  apiString += host;
  apiString += sensorId;
  
  Serial.begin(115200);
  SPIFFS.begin();

  File f = SPIFFS.open("/wifi.txt", "r");

  while(f.available())
  {
        //Lets read line by line from the file
        String line = f.readStringUntil('\n');
        Serial.println(line);
  }
  f.close();

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

void postJson() {
  String json; 
  serializeJson(postData, json);

  WiFiClientSecure client;
  client.setInsecure(); //the magic line, use with caution
  client.connect(host, httpsPort);
  
  HTTPClient http; //Object of class HTTPClient
  
  http.begin(client, apiString);

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
    postData["type"] = "humidity";
    postData["value"] = humidity;
    postData["connectKey"] = connectKey;

    postJson();
    
    //Begin second post
    postData["type"] = "temperature";
    postData["value"] = F;
    postData["connectKey"] = connectKey;

    postJson();
  }

  //delay 10 minutes
  delay(1000*30);
}
