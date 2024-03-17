#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include "Wire.h"

//DHT setup
#include "DHT.h"
#define DHTTYPE DHT20   // DHT 20
#define LED_BUILTIN A0
/*Notice: The DHT10 and DHT20 is different from other DHT* sensor ,it uses i2c interface rather than one wire*/
/*So it doesn't require a pin.*/
DHT dht(DHTTYPE);         //   DHT10 DHT20 don't need to define Pin

// const char* ssid = "OpenWrt-H68K";
// const char* password = "00000000";
const char* ssid = "Xiaomi_BACF";
const char* password = "Jiang15248193599";

//store temp and humi data
float h;
float t;

// API endpoint
const char* apiEndpoint = "https://rtch7hy6yk.execute-api.us-east-1.amazonaws.com/DHT20API";

void setup() {
  // Start the serial communication
  Serial.begin(115200);
  // debug.begin(115200);
  // debug.println("DHTxx test!");
  Wire.begin();

  /*if using WIO link,must pull up the power pin.*/
  // pinMode(PIN_GROVE_POWER, OUTPUT);
  // digitalWrite(PIN_GROVE_POWER, 1);

  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  // Once connected, print the IP address
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  //get temperature and humi data
    h = dht.readHumidity();
    t = dht.readTemperature();
    Serial.println("T&H: ");
    Serial.println(h);
    Serial.println(t);

  // Call your custom function here
  float inference = postData(h, t);
  if(inference>7.1){
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(5000);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    delay(1000);     
  }

  // Add some delay between HTTP requests
  delay(10000); // 10 seconds
}

float postData(float value1, float value2) {
  //recive result
  float result;
  // Check Wi-Fi connection status
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(apiEndpoint);
    http.addHeader("Content-Type", "application/json");

    // Create JSON array and object
    JSONVar myArray;
    myArray[0] = value1;
    myArray[1] = value2;
    
    JSONVar payload;
    payload["data"] = myArray;


    // Serialize JSON object to String
    String jsonString = JSON.stringify(payload);
    Serial.println("payload: " + jsonString);

    // Send the POST request
    int httpResponseCode = http.POST(jsonString);

    // If we receive a valid response, parse it
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
      result = response.toFloat();
    }
    else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    // End the connection
    http.end();
  }
  else {
    Serial.println("Error in WiFi connection");
  }
  return result;
}
