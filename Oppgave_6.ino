// Include libraries
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// Set OLED screen size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Define Oled states
#define HCSR  0
#define TMP   1
#define PHOTO 2
#define ALARM 3

// Set up display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Set up all pins used
const int photoRes = A0;
const int tempPin = A1;
const int triggerPin = 8;
const int echoPin = 7;
const int buttonPin = 2;
const int buzzPin = 11;

// Define diffrent values used troughout the code
int OLEDstate = 0;
int lastBtnState = 0;
int currBtnState;
unsigned long previousMillis = 0;
const long interval = 50;
int buzzerState = 0;

// Setup code
void setup() {
  // Start serial
  Serial.begin(9600);

  // Set up all pins
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT);
  pinMode(buzzPin, OUTPUT);

  // Check if display is connected
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Set display settings
  display.setTextSize(2);
  display.setTextColor(WHITE);
}

void loop() {
  // Read all the sensors
  float read_light = readVoltage(photoRes);
  float read_celsius = readTemp(tempPin);
  float read_distance = readDistance(triggerPin, echoPin);

  // Read the button
  currBtnState = digitalRead(buttonPin);

  // Set the current time
  unsigned long currentMillis = millis();

  // This if turns the buzzerState on and off every 50 milliseconds, used to beep the buzzer
  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    if (buzzerState == 0){
      buzzerState = 1;
    } else buzzerState = 0;
  }

  // This if checks if the button gets pressed, then if so, changes the OLEDstate
  if (currBtnState != lastBtnState){
    if (currBtnState == 1){
      OLEDstate += 1;
    }
    lastBtnState = currBtnState;
  }
  if (OLEDstate >= 4) OLEDstate = 0;

  // Clear and reset the display
  display.clearDisplay();
  display.setCursor(0, 0);

  // Defines text used on the screen
  String nmText;
  String valueText;

  // Checks witch value to send to the screen
  // Also runs the alarm system if this is selected
  switch (OLEDstate){
    case HCSR:
      nmText = "Distance:";
      valueText = String(read_distance) + " cm";
      break;
    case TMP:
      nmText = "Temp:";
      valueText = String(read_celsius) + " c";
      break;
    case PHOTO:
      nmText = "Light:";
      valueText = String(read_light) + " v";
      break;
    case ALARM:
      nmText = read_distance;
      valueText = "Alarm";

      // Checks if the distance is smaller than 20cm, and if so it beeps the buzzer and inverts the screen
      if (read_distance < 20.0){
        valueText = "Alarm";
         display.invertDisplay(true);
        if (buzzerState == 0){
          tone(buzzPin, 1000);
        } else {
          noTone(buzzPin);
        }
      } else {
        valueText = "No alarm";
        noTone(buzzPin);
        display.invertDisplay(false);
      }
      break;
  }

  // Send text to the screen
  display.println(nmText);
  display.println(valueText);
  display.display(); 
}

// Function to read value from photoresistor
// Takes in the pin connected to photoresistor
// Returns value in volts
float readVoltage(int pin){
  float readValue = analogRead(pin);
  return (readValue*5)/1023;
}

// Function to read the temprature
// Takes in the pin connected to temp sensor
// Returns value in celsius
float readTemp(int pin){
  float readValue = analogRead(pin);
  float voltage = (readValue*5)/1023;
  return (voltage-0.5)*100;
}

// Function to read teh distance sensor
// Takes in pin for trigger and echo
// Returns value in cm
float readDistance(int trigger, int echo){
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);

  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);

  long duration = pulseIn(echo, HIGH);

  return duration * 0.0343 / 2;
}
