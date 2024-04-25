#include <Arduino.h>
#include <SimpleDHT.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneButton.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C
#define RELAY_PIN 2
#define MANUAL_BTN 3
#define AUTOMATIC_BTN 4
#define RESET_BTN 5
#define DHT_PIN 7

// States
enum FanState {
  MANUAL,
  AUTOMATIC,
  AUTOMATIC_DEFAULT
};

// Events
enum FanEvent {
  MANUAL_BUTTON_CLICK,
  AUTOMATIC_BUTTON_CLICK,
};

// Variables
SimpleDHT11 sensorDHT(DHT_PIN);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, RESET_BTN);
int histeresis = 5;
int cutInHumidity = 60;
byte temperature = 0;
byte humidity = 0;
byte data[40] = {0};
bool fanSignal = false;
FanState currentState = AUTOMATIC_DEFAULT;
unsigned long lastManualActivationTime = 0;

// Functions
void read_sensor();
void handleEvent(FanEvent event);
void handleManualMode();
void handleAutomaticMode();
void handleAutomaticDefault();


void setup() {
  Serial.begin(9600);
  // PIN Config
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(MANUAL_BTN, INPUT_PULLUP);
  pinMode(AUTOMATIC_BTN, INPUT_PULLUP);
  pinMode(RESET_BTN, INPUT_PULLUP);


  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Hello, world!");
  display.display();
}

void loop() {
  if (digitalRead(MANUAL_BTN) == LOW)
  {
    handleEvent(MANUAL_BUTTON_CLICK);
  }
  else if (digitalRead(AUTOMATIC_BTN) == LOW)
  {
    handleEvent(AUTOMATIC_BUTTON_CLICK);
  }
  else
  {
    handleAutomaticDefault();
  }
}

void handleEvent(FanEvent event){
  switch (event)
  {
  case MANUAL_BUTTON_CLICK:
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("MANUAL");
    display.display();
    handleManualMode();
    break;
  case AUTOMATIC_BUTTON_CLICK:
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("AUTOMATIC");
    display.display();
    handleAutomaticMode();
    break;
  }
}

void handleManualMode(){
  switch (currentState)
  {
  case MANUAL:
    if (millis() - lastManualActivationTime >= 900000)
    {
      digitalWrite(RELAY_PIN, LOW);
      currentState = AUTOMATIC;
    }
    break;
  case AUTOMATIC:
    digitalWrite(RELAY_PIN, HIGH);
    lastManualActivationTime = millis();
    currentState = MANUAL;
    break;
  case AUTOMATIC_DEFAULT:
    digitalWrite(RELAY_PIN, HIGH);
    lastManualActivationTime = millis();
    currentState = MANUAL;
    break;
  }
}

void handleAutomaticMode(){
  switch (currentState)
  {
  case MANUAL:
    currentState = AUTOMATIC;
    break;
  case AUTOMATIC:
    read_sensor();
    delay(1000);
    break;
  case AUTOMATIC_DEFAULT:
    currentState = AUTOMATIC;
    break;
  }
}

void handleAutomaticDefault(){
  switch (currentState)
  {
  case MANUAL:
    break;
  case AUTOMATIC:
    break;
  case AUTOMATIC_DEFAULT:
    read_sensor();
    delay(1000);
    break;
  }
}

void read_sensor() {
  sensorDHT.read(&temperature, &humidity, data);
  
  if (cutInHumidity < (int) humidity)
  {
    digitalWrite(RELAY_PIN, HIGH);
  }
  else if ((cutInHumidity - (int) humidity) > histeresis)
  {
    digitalWrite(RELAY_PIN, LOW);
  }  
}

