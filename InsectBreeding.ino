#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Lab3";
const char* password = "123456lab78";

const char* serverUrl = "https://192.168.31.62:7202/api/LivingPlace/updateTH";

String newSSID;
String newPassword;
String apiKey;

ESP8266WebServer server(80);

#define DHTPIN 0  //pin where the dht11 is connected
DHT dht(DHTPIN, DHT11);


void setup() {
  Serial.begin(115200);
  delay(10);
  dht.begin();
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/save", handleSave);

  server.begin();
  Serial.println("Server started");
}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>Configure Wi-Fi</h1>";
  html += "<form method='POST' action='/save'>";
  html += "SSID: <input type='text' name='ssid'><br>";
  html += "Password: <input type='password' name='password'><br>";
  html += "API Key: <input type='text' name='apiKey'><br>";
  html += "<input type='submit' value='Save'>";
  html += "</form>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  newSSID = server.arg("ssid");
  newPassword = server.arg("password");
  apiKey = server.arg("apiKey");

  server.send(200, "text/html", "Wi-Fi configuration saved. Restarting...");
  delay(1000);
  
  WiFi.softAPdisconnect(true);
  
  WiFi.begin(newSSID.c_str(), newPassword.c_str());
  
  int timeout = 10;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(1000);
    timeout--;
    Serial.println("Connecting to WiFi...");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  server.handleClient();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
  }

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" degrees Celcius, Humidity: ");
  Serial.print(h);

  //Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) {
    
    Serial.print(" ApiKey: ");  
    Serial.print(apiKey); 

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    http.begin(client, serverUrl);

    http.addHeader("Content-Type", "application/json"); 

    // Send HTTP POST request
    int httpResponseCode = http.POST("{\"livingPlaceId\":\"" + apiKey + "\",\"Temperature\":" + String(t) + ",\"Humidity\":" + String(h) + "}");

    Serial.print(" -> HTTP Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode != HTTP_CODE_OK) {
      Serial.print("HTTP Error: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    // Free resources
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }


  delay(5000);
}
