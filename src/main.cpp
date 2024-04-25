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
#define ON_UP_BTN 3
#define OFF_DOWN_BTN 4
#define SELECT_BTN 5
#define RESET_BTN 8
#define DHT_PIN 7

// States
enum FanState {
  OFF,
  MANUAL,
  AUTOMATIC
};

// Events
enum FanEvent {
  MANUAL_BUTTON_CLICK,
  AUTOMATIC_BUTTON_CLICK,
  OFF_BUTTON_CLICK,
  MANUAL_MODE_END
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
FanState currentState = OFF;


// Functions
void read_sensor();
void check_state();
void run_histerisis();




void setup() {
  Serial.begin(9600);
  // PIN Config
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(ON_UP_BTN, INPUT_PULLUP);
  pinMode(OFF_DOWN_BTN, INPUT_PULLUP);
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

  check_state();
  read_sensor();
  run_histerisis();

  delay(1000);
}

void read_sensor() {
  if(sensorDHT.read(&temperature, &humidity, data)){
    return;
  }
  
  if (cutInHumidity < (int) humidity)
  {
    fanSignal = true;
  }
  else if ((cutInHumidity - (int) humidity) > histeresis)
  {
    fanSignal = false;
  }  
}

void check_state() {
  if ((digitalRead(ON_UP_BTN) == LOW) && !fanSignal)
  {
    digitalWrite(RELAY_PIN, HIGH);
    delay(15000);
  }
  else if ((digitalRead(OFF_DOWN_BTN) == LOW) && !fanSignal)
  {
    digitalWrite(RELAY_PIN, LOW);
  }
  else if (fanSignal)
  {
    run_histerisis();
  }
}

void run_histerisis() {
    if (fanSignal)
  {
    digitalWrite(RELAY_PIN, HIGH);
  }
  else
  {
    digitalWrite(RELAY_PIN, LOW);
  }
}