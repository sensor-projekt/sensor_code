//adapted from https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/BearSSL_Validation/BearSSL_Validation.ino
#include <WiFiClientSecure.h> 
#include <ArduinoJson.h>
#include "DHTesp.h" //https://github.com/beegee-tokyo/DHTesp
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <time.h>
#include "certs.h"

DHTesp dht;

const char* apiString = "https://sensorproject.xyz/postmeasurement/";
const char* sensorAuth1 = "Authorization: Basic NDg5ZjdiYjYtNTVhZC00OWI4LWI1MTctMzg2ZDhmZjM4NmEzOnhacSYwME8tNjA=";
const char* sensorId1 = "489f7bb6-55ad-49b8-b517-386d8ff386a3";
const char* sensorKey = "xZq&00O-60";
const char* sensorAuth2 = "Authorization:Basic NjlmYTk0MWMtMWFiNy00Y2Q2LWE1OTYtY2E4YzE0Yzk1YzdiOnhacSYwME8tNjA=";
const char* sensorId2 = "69fa941c-1ab7-4cd6-a596-ca8c14c95c7b";

DynamicJsonDocument postData(256);
DynamicJsonDocument responseData(256);
const char* host = "sensorproject.xyz";
const uint16_t port = 443;

#define CERT GTS_CA_1D4

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
  // wifiManager.resetSettings();
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect(sensorKey);
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  //shutdown access point
  wifiManager.stopWebPortal();
}

// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void postJson(bool sensorAuth) {
  String bodyString; 
  serializeJson(postData, bodyString);

  BearSSL::WiFiClientSecure client;
  BearSSL::X509List cert(CERT);
  client.setTrustAnchors(&cert);
  setClock();

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

  delay 5 minutes
  delay(1000*5*60);

}
