#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h> 
#include <ArduinoJson.h>
#include "DHTesp.h" //https://github.com/beegee-tokyo/DHTesp
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

DHTesp dht;

const char* apiString = "https://sensorproject.xyz/postmeasurement/";
const char* sensorAuth1 = "Basic ZDZmYjUwYTAtODNhOC00NjRhLTljZmQtM2JjZTkyY2Y4ZDJmOlF4JDgzLDYyV3o=";
const char* sensorId1 = "d6fb50a0-83a8-464a-9cfd-3bce92cf8d2f";
const char* sensorKey1 = "Qx$83,62Wz";
const char* sensorAuth2 = "Basic MDgwZmI4ODktMDU3YS00ZDY5LTlkNWUtZWIxNTQ1Y2Q4MjBiOlF4JDgzLDYyV3o=";
const char* sensorId2 = "5299f177-1ecd-4c65-9ed0-33039d3ce1a9";
const char* sensorKey2 = "Qx$83,62Wz";

DynamicJsonDocument postData(256);
DynamicJsonDocument responseData(256);
const char* host = "www.sensorproject.xyz";
const uint16_t port = 443;

// CN: sni.cloudflaressl.com => name: sni_cloudflaressl_com
// not valid before: 2021-09-11 00:00:00
// not valid after:  2022-09-10 23:59:59
const char* fingerprint = "CC:24:49:F2:77:70:71:94:C7:51:56:EC:44:FB:0C:67:AC:67:F7:39";


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
  wifiManager.autoConnect(sensorKey1);
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  //shutdown access point
  wifiManager.stopWebPortal();
}

// Try and connect using a WiFiClientBearSSL to specified host:port and dump HTTP response
void fetchURL(BearSSL::WiFiClientSecure *client, const char *host, const uint16_t port, const char *path) {
  if (!path) { path = "/"; }

  ESP.resetFreeContStack();
  uint32_t freeStackStart = ESP.getFreeContStack();
  Serial.printf("Trying: %s:443...", host);
  client->connect(host, port);
  if (!client->connected()) {
    Serial.printf("*** Can't connect. ***\n-------\n");
    return;
  }
  Serial.printf("Connected!\n-------\n");
  client->write("GET ");
  client->write(path);
  client->write(" HTTP/1.0\r\nHost: ");
  client->write(host);
  client->write("\r\nUser-Agent: ESP8266\r\n");
  client->write("\r\n");
  uint32_t to = millis() + 5000;
  if (client->connected()) {
    do {
      char tmp[32];
      memset(tmp, 0, 32);
      int rlen = client->read((uint8_t *)tmp, sizeof(tmp) - 1);
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
  client->stop();
  uint32_t freeStackEnd = ESP.getFreeContStack();
  Serial.printf("\nCONT stack used: %d\n", freeStackStart - freeStackEnd);
  Serial.printf("BSSL stack used: %d\n-------\n\n", stack_thunk_get_max_usage());
}


void fetchInsecure() {
  Serial.printf(R"EOF(
This is absolutely *insecure*, but you can tell BearSSL not to check the
certificate of the server.  In this mode it will accept ANY certificate,
which is subject to man-in-the-middle (MITM) attacks.
)EOF");
  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  fetchURL(&client, host, port, "/greeting");
}


void postJson(String sensorAuth) {
  String json; 
  serializeJson(postData, json);

  Serial.print("json: ");
  Serial.println(json);

  fetchInsecure();

//  if (client.verify("29 70 30 74 CA 3C 48 F5 4A 79 C6 2D 11 57 A2 412A 2D 7D 5C", "api.github.com")) 
//  {  
//    HTTPClient http;  
//    http.begin(client, "https://sensorproject.xzy");
//
//    String payload;
//    if (http.GET() == HTTP_CODE_OK)    
//        Serial.println("connection successful...");   
//    HTTPClient http;  
//    http.begin(client, apiString);
//    http.addHeader("Content-Type", "application/json");
//    http.addHeader("Authorization", sensorAuth.c_str());
//    int httpCode = http.POST(json);
//  
//    Serial.print("httpCode: ");
//    Serial.println(httpCode);
//    
//    if (httpCode > 0) {
//      if (httpCode != 200 && httpCode != 201) {
//        String response = http.getString();
//        DeserializationError error = deserializeJson(responseData, response);
//        if (error) {
//          Serial.print(F("deserializeJson() failed: "));
//          Serial.println(error.c_str());
//        }
//        Serial.print("Something went wrong: ");
//        Serial.println(response);
//        Serial.println("");
//      }
//      else {
//        String response = http.getString();
//        Serial.print("response: ");
//        Serial.println(response);
//        Serial.println("");
//      }
//    }
//    http.end(); //Close connection
//  }
//  else 
//  {
//    Serial.println("certificate doesn't match");
//  }
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
  delay(1000*50*60);
}
