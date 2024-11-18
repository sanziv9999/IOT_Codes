#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <espnow.h>
#include <DHT.h>

// const char *ssid = "sanjeevmagar3575";
// const char *password = "53065306SanzivXM";

const char *ssid = "Virinchi College";
const char *password = "virinchi@2024";

// GPIO pins for devices
const int fanPin = 14;           // D5
const int bedroomLightPin = 12;  // D6
const int livingRoomLightPin = 13; // D7
const int kitchenLightPin = 15;  // D8
const int testPin = 4;           // D2
const int buzzer = 0; // D3
4
// DHT Sensor configuration
const int DHTPin = 5;   // D1
const int DHTType = DHT11;   // DHT11 or DHT22
DHT dht(DHTPin, DHTType);

// MQ2 Gas Sensor pin
const int gasSensorPin = A0;  // Analog pin for MQ2 gas sensor

// Initialize WebServer on port 80
ESP8266WebServer server(80);

// MAC address of the ESP-NOW sender
uint8_t senderMacAddress[] = {0xBC, 0xFF, 0x4D, 0x5F, 0x60, 0xC9};

// Water level storage
int waterLevel = 0;

// Structure to hold received data
typedef struct struct_message {
    float distance; // Change to float to match the sender's structure
} struct_message;

struct_message incomingData;

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Setup GPIO pins
    pinMode(fanPin, OUTPUT);
    pinMode(bedroomLightPin, OUTPUT);
    pinMode(livingRoomLightPin, OUTPUT);
    pinMode(kitchenLightPin, OUTPUT);
    pinMode(testPin, OUTPUT);
    pinMode(buzzer, OUTPUT);

    // Set initial states
    digitalWrite(fanPin, LOW);
    digitalWrite(bedroomLightPin, LOW);
    digitalWrite(livingRoomLightPin, LOW);
    digitalWrite(kitchenLightPin, LOW);
    digitalWrite(testPin, LOW);
    digitalWrite(buzzer,LOW);

    // Start DHT sensor
    dht.begin();

    // Initialize ESP-NOW
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register peer and set receive callback
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_add_peer(senderMacAddress, ESP_NOW_ROLE_CONTROLLER, 1, NULL, 0);
    esp_now_register_recv_cb(onDataRecv);  // Register callback for receiving data

    // Setup web server routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/fan/toggle", HTTP_GET, handleFanToggle);
    server.on("/bedroom/toggle", HTTP_GET, handleBedroomLightToggle);
    server.on("/living/toggle", HTTP_GET, handleLivingRoomLightToggle);
    server.on("/kitchen/toggle", HTTP_GET, handleKitchenLightToggle);
    server.on("/test/toggle", HTTP_GET, handleTestToggle);

    server.begin();
}

void loop() {
    server.handleClient();
    readDHT();
}

// Handle ESP-NOW data reception
void onDataRecv(uint8_t *mac, uint8_t *incomingDataPtr, uint8_t len) {
    Serial.print("Data received from: ");
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // Cast incoming data to struct_message
    memcpy(&incomingData, incomingDataPtr, sizeof(incomingData));

    // Access the distance
    float distance = incomingData.distance;  // Updated to use float
    Serial.print("Distance received: ");
    Serial.println(distance);

    // Define the maximum distance (empty tank) and calculate the water level percentage
    int maxDistance = 10;  // Adjust this value to your sensor's maximum distance
    waterLevel = map(distance, 0, maxDistance, 100, 0);  // Map distance to percentage

    Serial.print("Water Level: ");
    Serial.println(waterLevel);
}

// Handle the root page
void handleRoot() {
    int gasLevel = analogRead(gasSensorPin);  // Read gas sensor value

    // Check if the gas level exceeds the threshold and trigger the buzzer
    if (gasLevel > 300) {
        digitalWrite(buzzer, HIGH);  // Turn on the buzzer
    } else {
        digitalWrite(buzzer, LOW);   // Turn off the buzzer
    }

    String html = "<html><head><style>";
    html += "body { display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; margin: 0; }";
    html += ".sensor-box { display: flex; flex-direction: column; align-items: center; justify-content: center; border: 2px solid #ccc; padding: 10px; margin-bottom: 20px; width: 90%; max-width: 400px; }";
    html += ".button-box { display: flex; flex-direction: column; align-items: center; justify-content: center; border: 2px solid #ccc; padding: 10px; width: 90%; max-width: 400px; }";
    html += ".button-box h1 { font-size: 1.5em; margin-bottom: 10px; }";
    html += ".button-row { display: flex; justify-content: space-between; align-items: center; width: 100%; margin: 5px 0; }";
    html += ".button-row p { font-size: 1.2em; flex-grow: 1; margin: 0; }";
    html += "button { font-size: 1em; padding: 8px 16px; }";

    // Add responsive styles for smaller screens
    html += "@media (max-width: 600px) {";
    html += "body { font-size: 1.2em; }";
    html += ".sensor-box h1, .button-box h1 { font-size: 1.2em; }";
    html += ".button-row p { font-size: 1em; }";
    html += "button { font-size: 1.2em; padding: 10px 20px; }";
    html += "}";

    html += "</style></head><body>";

    // Display temperature and humidity
    html += "<div class='sensor-box'>";
    html += "<h1>Temperature and Humidity</h1>";
    html += "<p>Temperature: " + String(getTemperature()) + " &#8451;</p>";
    html += "<p>Humidity: " + String(getHumidity()) + " %</p>";
    html += "</div>";

    // Display water level with progress bar
    html += "<div class='sensor-box'>";
    html += "<h1>Water Level</h1>";
    html += "<div style='width: 100%; background-color: #ddd;'>";
    html += "<div style='width: " + String(waterLevel) + "%; height: 30px; background-color: #4CAF50;'></div>";
    html += "</div>";
    html += "<p>Water Level: " + String(waterLevel) + " %</p>";
    html += "</div>";

    // Display gas sensor level
    html += "<div class='sensor-box'>";
    html += "<h1>Gas Sensor</h1>";
    html += "<p>Gas Level: " + String(gasLevel) + "</p>";
    html += "</div>";

    // Buttons for controlling devices with right-aligned buttons
    html += "<div class='button-box'>";
    html += "<h1>ESP8266 Home Automation</h1>";
    html += "<div class='button-row'><p>Fan:</p><button id='fan-btn' onclick='toggleFan()'>" + getFanState() + "</button></div>";
    html += "<div class='button-row'><p>Bedroom Light:</p><button id='bedroom-light-btn' onclick='toggleBedroomLight()'>" + getBedroomLightState() + "</button></div>";
    html += "<div class='button-row'><p>Living Room Light:</p><button id='living-light-btn' onclick='toggleLivingRoomLight()'>" + getLivingRoomLightState() + "</button></div>";
    html += "<div class='button-row'><p>Kitchen Light:</p><button id='kitchen-light-btn' onclick='toggleKitchenLight()'>" + getKitchenLightState() + "</button></div>";
    html += "<div class='button-row'><p>Test Button:</p><button id='test-btn' onclick='toggleTest()'>" + getTestState() + "</button></div>";
    html += "</div>";

    html += "<script>";
    html += "function toggleFan() { fetch('/fan/toggle').then(reloadPage); }";
    html += "function toggleBedroomLight() { fetch('/bedroom/toggle').then(reloadPage); }";
    html += "function toggleLivingRoomLight() { fetch('/living/toggle').then(reloadPage); }";
    html += "function toggleKitchenLight() { fetch('/kitchen/toggle').then(reloadPage); }";
    html += "function toggleTest() { fetch('/test/toggle').then(reloadPage); }";
    html += "function reloadPage() { location.reload(); }";
    html += "setInterval(reloadPage, 3000);"; // Reload page every 3 seconds
    html += "</script>";

    html += "</body></html>";

    server.send(200, "text/html", html);
}





// Functions to handle toggling devices
void handleFanToggle() {
    digitalWrite(fanPin, !digitalRead(fanPin));
    server.send(200, "text/plain", getFanState());
}

void handleBedroomLightToggle() {
    digitalWrite(bedroomLightPin, !digitalRead(bedroomLightPin));
    server.send(200, "text/plain", getBedroomLightState());
}

void handleLivingRoomLightToggle() {
    digitalWrite(livingRoomLightPin, !digitalRead(livingRoomLightPin));
    server.send(200, "text/plain", getLivingRoomLightState());
}

void handleKitchenLightToggle() {
    digitalWrite(kitchenLightPin, !digitalRead(kitchenLightPin));
    server.send(200, "text/plain", getKitchenLightState());
}

void handleTestToggle() {
    digitalWrite(testPin, !digitalRead(testPin));
    server.send(200, "text/plain", getTestState());
}

// DHT functions
void readDHT() {
    // You can call this in the loop to continuously get temperature and humidity values
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
}

float getTemperature() {
    return dht.readTemperature();
}

float getHumidity() {
    return dht.readHumidity();
}

// Functions to get states of devices
String getFanState() {
    return digitalRead(fanPin) ? "OFF" : "ON";
}

String getBedroomLightState() {
    return digitalRead(bedroomLightPin) ? "OFF" : "ON";
}

String getLivingRoomLightState() {
    return digitalRead(livingRoomLightPin) ? "OFF" : "ON";
}

String getKitchenLightState() {
    return digitalRead(kitchenLightPin) ? "OFF" : "ON";
}

String getTestState() {
    return digitalRead(testPin) ? "OFF" : "ON";
}
