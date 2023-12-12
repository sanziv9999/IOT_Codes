int buzzerPin = 4;  // Pin for the buzzer
int redLEDPin = 2;  // Pin for the red LED
int handlePin = 7;  // Pin for the handle
int minFrequency = 500; // Minimum frequency of the siren
int maxFrequency = 2000; // Maximum frequency of the siren
int frequencyStep = 500; // Frequency step for changing the tone
int delayTime = 10; // Delay time between frequency steps

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
