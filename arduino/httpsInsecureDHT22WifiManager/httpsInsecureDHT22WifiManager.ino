#include <WiFiClientSecure.h> 
#include <ArduinoJson.h>
#include "DHTesp.h" //https://github.com/beegee-tokyo/DHTesp
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

DHTesp dht;

const char* apiString = "https://sensorproject.xyz/postmeasurement/";
const char* sensorAuth1 = "Authorization: Basic ZDZmYjUwYTAtODNhOC00NjRhLTljZmQtM2JjZTkyY2Y4ZDJmOlF4JDgzLDYyV3o=";
const char* sensorId1 = "d6fb50a0-83a8-464a-9cfd-3bce92cf8d2f";
const char* sensorKey = "Qx$83,62Wz";
const char* sensorAuth2 = "Authorization:Basic MDgwZmI4ODktMDU3YS00ZDY5LTlkNWUtZWIxNTQ1Y2Q4MjBiOlF4JDgzLDYyV3o=";
const char* sensorId2 = "080fb889-057a-4d69-9d5e-eb1545cd820b";
//const char* fingerprint = "CC:24:49:F2:77:70:71:94:C7:51:56:EC:44:FB:0C:67:AC:67:F7:39";
DynamicJsonDocument postData(256);
DynamicJsonDocument responseData(256);
const char* host = "sensorproject.xyz";
const uint16_t port = 443;

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  
  //setup DHT22 sensor
  dht.setup(D2, DHTesp::DHT22);
  
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // Uncomment and run it once, if you want to erase all the stored information
  wifiManager.resetSettings();DynamicJsonDocument bodyData(256);
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect(sensorKey);
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  //shutdown access point
  wifiManager.stopWebPortal();
}

void postJson(bool sensorAuth) {
  String bodyString; 
  serializeJson(postData, bodyString);

  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  //client.setFingerprint(fingerprint);
  Serial.printf("\nTrying: %s:443...", host);
  client.connect(host, port);
  if (!client.connected()) {
    Serial.printf("*** Can't connect. ***\n-------\n");
    return;
  }
  Serial.printf("Connected!\n-------\n");
  client.print("POST ");
  client.print("/postmeasurement/");
  client.print(" HTTP/1.0\r\nHost: ");
  client.print(host);
  client.print("\r\n");

  sensorAuth ?
  client.print(sensorAuth1)
  : client.print(sensorAuth2);  
  
  client.print("\r\n");
  client.print("Content-Type: application/json");
  client.print("\r\n");
  client.print("Content-Length: "); 
  client.print(strlen(bodyString.c_str()));
  client.print("\r\n");
  client.print("\r\n");
  client.print(bodyString);
  uint32_t to = millis() + 5000;
  if (client.connected()) {
    do {
      char tmp[32];
      memset(tmp, 0, 32);
      int rlen = client.read((uint8_t *)tmp, sizeof(tmp) - 1);
      yield();
      if (rlen < 0) { break; }
      // Only print out first line up to \r, then abort connection
      char *nl = strchr(tmp, '\r');
      if (nl) {
        *nl = 0;
        Serial.print(tmp);
        break;
      }
      Serial.print(tmp);
    } while (millis() < to);
  }
  client.stop();
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

    postJson(true);
    
    postData["sensorId"] = sensorId2;
    postData["value"] = humidity;

    postJson(false);

  }

  //delay 5 minutes
  delay(1000*5*60);
}
