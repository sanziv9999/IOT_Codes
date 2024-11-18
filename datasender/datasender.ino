#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Servo.h>

// Receiver MAC Address
uint8_t receiverMac[] = {0x48, 0x55, 0x19, 0xE0, 0x18, 0xBE};

// Structure to send data
typedef struct struct_message {
    float distance; // Distance in cm
} struct_message;

struct_message myData;



// Define ultrasonic sensor pins
const int trigPin = D1;
const int echoPin = D2;

// Motor control pin
const int motorPin = D5; // Use an appropriate GPIO pin for controlling the water motor

// Raindrop sensor pin and threshold
const int raindropPin = A0; // Analog pin for raindrop sensor
const int rainThreshold = 500; // Adjust based on sensor calibration

// Servo motor for rain response
Servo rainServo;
const int servoPin = D6; // Pin connected to SPT320 servo motor

// Timeout value for pulseIn (in microseconds)
const long echoTimeout = 30000; // 30ms (around 5 meters)

// Define the water level range (in cm) for motor control
const float maxWaterLevel = 10.0; // Max water level in cm
const float minWaterLevel = 3.0;  // Minimum water level in cm

void onSent(uint8_t *mac_addr, uint8_t sendStatus) {
    Serial.print("Send Status: ");
    Serial.println(sendStatus == 0 ? "Success" : "Fail");
}

// Function to read distance from ultrasonic sensor
float readUltrasonicDistance() {
    // Clear the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Set the trigPin HIGH for 10 microseconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Read the echoPin, returns the sound wave travel time in microseconds
    long duration = pulseIn(echoPin, HIGH, echoTimeout); // Add timeout
    if (duration == 0) {
        // No signal received, return -1 as an error indicator
        return -1;
    }
    
    // Calculate the distance in cm (speed of sound is 340 m/s)
    float distance = duration * 0.034 / 2; // distance = time * speed / 2
    return distance;
}

void setup() {
    Serial.begin(115200);

    // Initialize pins
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(motorPin, OUTPUT); // Initialize motor control pin as output
    digitalWrite(motorPin, LOW); // Ensure motor is initially off

    // Initialize raindrop sensor pin
    pinMode(raindropPin, INPUT);

    // Initialize servo
    rainServo.attach(servoPin);
    rainServo.write(0); // Start with servo at 0 degrees

    // Initialize Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Initialize ESP-NOW
    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register send callback
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(onSent);

    // Add peer (Receiver)
    esp_now_add_peer(receiverMac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
    // Ultrasonic sensor distance reading
    float distance = readUltrasonicDistance();

    if (distance != -1) {
        myData.distance = distance; // Update data only if valid

        // Control water motor based on water level (distance)
        if (distance >= minWaterLevel && distance <= maxWaterLevel) {
            digitalWrite(motorPin, HIGH); // Turn on the motor
            Serial.println("Water Motor ON");
        } else {
            digitalWrite(motorPin, LOW); // Turn off the motor
            Serial.println("Water Motor OFF");
        }

        // Send data
        esp_now_send(receiverMac, (uint8_t *)&myData, sizeof(myData));
        Serial.print("Distance sent: ");
        Serial.println(myData.distance);
    } else {
        Serial.println("No signal received, skipping data send.");
        digitalWrite(motorPin, LOW); // Ensure motor is off if no valid data
    }

    // Raindrop sensor reading
    int rainValue = analogRead(raindropPin);
    if (rainValue < rainThreshold) {
        rainServo.write(180); // Rotate servo to 180 degrees when rain is detected
        Serial.println("Rain detected, servo rotated to 180 degrees.");
    } else {
        rainServo.write(0); // Reset servo to 0 degrees when no rain
        Serial.println("No rain, servo at 0 degrees.");
    }

    delay(100); // Adjust delay as needed
}
