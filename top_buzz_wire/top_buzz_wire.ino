int buzzerPin = 14;  // GPIO14 for the buzzer
int redLEDPin = 12;  // GPIO12 for the red LED
int handlePin = 2;  // GPIO2 for the handle
int minFrequency = 1000; // Minimum frequency of the siren
int maxFrequency = 2000; // Maximum frequency of the siren
int frequencyStep = 50; // Frequency step for changing the tone
int delayTime = 50; // Delay time between frequency steps

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);
  pinMode(handlePin, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(handlePin) == LOW) {
      // Maze loop touched
      digitalWrite(buzzerPin, HIGH);
      digitalWrite(redLEDPin, HIGH);
      playSiren();
      
    } else {
      // Handle touched but not the maze loop
      digitalWrite(buzzerPin, LOW);
      digitalWrite(redLEDPin, LOW);
    }
}

void playSiren() {
  // Increase frequency
  for (int frequency = minFrequency; frequency <= maxFrequency; frequency += frequencyStep) {
    tone(buzzerPin, frequency, delayTime);
    delay(delayTime);
  }

  // Decrease frequency
  for (int frequency = maxFrequency; frequency >= minFrequency; frequency -= frequencyStep) {
    tone(buzzerPin, frequency, delayTime);
    delay(delayTime);
  }

  noTone(buzzerPin); // Stop the tone
}
// 42,45,46

