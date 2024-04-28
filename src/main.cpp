#include <Arduino.h>
#include <SimpleDHT.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneButton.h>
#include <robotoMedium.h>
#include <robotoSmall.h>
#include <robotoNormal.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C
#define RELAY_PIN 2
#define MANUAL_BTN 3
#define AUTOMATIC_BTN 4
#define DHT_PIN 7

static const unsigned char hysteresis_bits[] PROGMEM = {
    0xf8, 0x07, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00,
    0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0xff, 0x00};

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
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
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
void draw();


void setup() {
  // PIN Config
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(MANUAL_BTN, INPUT_PULLUP);
  pinMode(AUTOMATIC_BTN, INPUT_PULLUP);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
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
  draw();
}

void handleEvent(FanEvent event){
  switch (event)
  {
  case MANUAL_BUTTON_CLICK:
    handleManualMode();
    break;
  case AUTOMATIC_BUTTON_CLICK:
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
    currentState = AUTOMATIC_DEFAULT;
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

void draw(){
  display.clearDisplay();
  display.drawLine(80, 46, 128, 46, 1);
  display.drawLine(80, 0, 80, 64, 1);
  display.setCursor(0, 22);
  display.setFont(&Roboto_Mono_Medium_30);
  display.print("H:");
  display.print((int) humidity);
  display.setCursor(0, 55);
  display.print("T:");
  display.print((int) temperature);
  display.setFont(&Roboto_Mono_Medium_9);
  display.setCursor(74, 9);
  display.print("%");
  display.setCursor(74, 40);
  display.print("O");
  display.setCursor(120, 9);
  display.print("%");
  display.setFont(&Roboto_Mono_Medium_30);
  display.setCursor(85, 22);
  display.print(cutInHumidity);
  display.setFont(&Roboto_Mono_Medium_10);
  display.setCursor(85, 40);
  display.print("Diff:");
  display.print(histeresis);
  switch (currentState)
  {
  case MANUAL:
    display.setCursor(85, 58);
    display.print("Manual");
    break;
  case AUTOMATIC:
    display.setCursor(85, 58);
    display.print("Auto");
    break;
  case AUTOMATIC_DEFAULT:
    display.setCursor(85, 58);
    display.print("Auto");
    break;
  }
  display.display();
}