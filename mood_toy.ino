// Hardware: Arduino Nano
//   RGB LED module (V R B G, common anode: LOW = on)
//   push button module (VCC GND + signal)
//   light sensor module (AO DO GND VCC, DO not used)

const int buttonPin = 2;   // the number of the pushbutton pin
const int RledPin = 3;     // the number of the R pin
const int GledPin = 4;     // the number of the G pin
const int BledPin = 5;     // the number of the B pin
const int ldrPin = A0;     // NEW: light sensor module AO pin

// variables will change:
int buttonState = 0;       // variable for reading the pushbutton status
int ledState = LOW;
bool ButtonPressed = false;
int buttonIdle = LOW;      // NEW: button reading when not pressed (measured in setup)

unsigned long previousMillis = 0; // will store last time LED was updated
unsigned long previousMillis2 = 0; // will store last time LED was updated
const long interval = 1000; // interval at which to blink (milliseconds)
const long Gametime = 5000;
const long result = 3000;
bool Gamestart = false;
int hits = 0;
int color = 0;

// NEW: mood settings (calibrate the two thresholds with the Serial Monitor)
const int darkThreshold = 300;       // light reading below this = dark
const int veryDarkThreshold = 100;   // light reading below this = very dark
const long darkDelay = 3000;         // dark for longer than this -> mood drops faster
const long slowDropInterval = 2000;  // dark: -1 every 2 s
const long fastDropInterval = 1000;  // dark > 3 s: -1 every 1 s, very dark: -2 every 1 s
const int maxMood = 10;
int mood = maxMood;
unsigned long darkStart = 0;         // when it became dark
unsigned long lastMoodDrop = 0;      // last time mood went down

void setup() {
  // initialize the LED pin as an output:
  pinMode(RledPin, OUTPUT);
  pinMode(GledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  buttonIdle = digitalRead(buttonPin);  // NEW: works whether the module reads HIGH or LOW when pressed
  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long currentMillis2 = millis();
  buttonState = digitalRead(buttonPin) == buttonIdle ? LOW : HIGH;  // HIGH = pressed
  int light = 1023 - analogRead(ldrPin);   // NEW: module reads lower in bright light, so flip it (higher = brighter)
  //Serial.println(light);          // uncomment to calibrate the thresholds

  // NEW: mood goes down while it is dark
  if (light >= darkThreshold) {
    // bright: mood stays, dark timer keeps resetting
    darkStart = currentMillis;
    lastMoodDrop = currentMillis;
  }
  else {
    unsigned long dropInterval = slowDropInterval;
    int dropAmount = 1;
    if (light < veryDarkThreshold) {                   // very dark: -2 every second
      dropInterval = fastDropInterval;
      dropAmount = 2;
    }
    else if (currentMillis - darkStart > darkDelay) {  // dark > 3 s: faster
      dropInterval = fastDropInterval;
    }
    if (currentMillis - lastMoodDrop >= dropInterval) {
      lastMoodDrop = currentMillis;
      mood = max(mood - dropAmount, 0);
      Serial.print("mood: ");
      Serial.println(mood);
    }
  }

  if (Gamestart == false){
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      if (ledState == LOW) {
        ledState = HIGH;
      }
      else {
        ledState = LOW;
      }
    }

    if (light < darkThreshold) {
      // NEW: dark -> LED shows the mood, button = pet the toy
      if (mood >= 7) {              // happy: green
        digitalWrite(RledPin, HIGH);
        digitalWrite(GledPin, LOW);
        digitalWrite(BledPin, HIGH);
      }
      else if (mood >= 4) {         // sad: blue
        digitalWrite(RledPin, HIGH);
        digitalWrite(GledPin, HIGH);
        digitalWrite(BledPin, LOW);
      }
      else {                        // very sad: blinking red
        digitalWrite(RledPin, ledState);
        digitalWrite(GledPin, HIGH);
        digitalWrite(BledPin, HIGH);
      }

      if (buttonState == HIGH && !ButtonPressed) {
        ButtonPressed = true;
        mood = min(mood + 1, maxMood);
        Serial.print("pet! mood: ");
        Serial.println(mood);
      }
      else if (buttonState == LOW && ButtonPressed) {
        ButtonPressed = false;
      }
    }
    else {
      // bright -> normal behavior
      if (color == 1){
        if (ledState == LOW) {
          digitalWrite(RledPin, LOW);
          digitalWrite(GledPin, HIGH);
          digitalWrite(BledPin, HIGH);
        }
        else {
          digitalWrite(RledPin, HIGH);
          digitalWrite(BledPin, HIGH);
          digitalWrite(GledPin, HIGH);
        }
      }
      else if (color == 2) {
        if (ledState == LOW) {
          digitalWrite(RledPin, HIGH);
          digitalWrite(GledPin, HIGH);
          digitalWrite(BledPin, LOW);
        }
        else {
          digitalWrite(RledPin, HIGH);
          digitalWrite(GledPin, HIGH);
          digitalWrite(BledPin, HIGH);
        }
      }
      else if (color == 3) {
        if (ledState == LOW) {
          digitalWrite(RledPin, HIGH);
          digitalWrite(GledPin, LOW);
          digitalWrite(BledPin, HIGH);
        }
        else {
          digitalWrite(RledPin, HIGH);
          digitalWrite(GledPin, HIGH);
          digitalWrite(BledPin, HIGH);
        }
      }
      else if (color == 0) {
        if (ledState == LOW) {
          digitalWrite(RledPin, LOW);
          digitalWrite(GledPin, LOW);
          digitalWrite(BledPin, LOW);
        }
        else {
          digitalWrite(RledPin, HIGH);
          digitalWrite(GledPin, HIGH);
          digitalWrite(BledPin, HIGH);
        }
      }

      // start the game on button release (now only checked while no game is running)
      if (buttonState == HIGH && !ButtonPressed) {
        ButtonPressed = true;
      }
      else if (buttonState == LOW && ButtonPressed){
        ButtonPressed = false;
        previousMillis = currentMillis;
        hits = 0;
        digitalWrite(RledPin, HIGH);
        digitalWrite(GledPin, HIGH);
        digitalWrite(BledPin, HIGH);
        Gamestart = true;
      }
    }
  }
  else {
    if (currentMillis - previousMillis >= Gametime) {
      if (currentMillis2 - previousMillis2 >= result) {
        Gamestart = false;
      }
      else{
        digitalWrite(RledPin, LOW);
        digitalWrite(GledPin, LOW);
        digitalWrite(BledPin, LOW);
        if (hits <= 10){
          color = 1;
        }
        else if (10 < hits && hits <= 20){
          color = 2;
        }
        else if (hits >20){
          color = 3;
        }
      }
    }
    else {
      //Serial.println("gaming_else");
      previousMillis2 = currentMillis2;
      if(buttonState == HIGH && ButtonPressed == false) {
        ButtonPressed = true;
        hits = hits + 1 ;
        Serial.println(hits);
        digitalWrite(RledPin, LOW);
        digitalWrite(GledPin, HIGH);
        digitalWrite(BledPin, HIGH);
      }
      else if (buttonState == LOW && ButtonPressed == true){
        ButtonPressed = false;
        digitalWrite(RledPin, HIGH);
        digitalWrite(GledPin, HIGH);
        digitalWrite(BledPin, HIGH);
      }
    }
  }
}
