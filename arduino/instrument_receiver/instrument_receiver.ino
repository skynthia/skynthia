#include <SoftwareSerial.h>
#include <FastLED.h>
#define PIN       2
#define NUM_LEDS  4

SoftwareSerial HC12(10, 11); // HC-12 TX Pin, HC-12 RX Pin
CRGB leds[NUM_LEDS];

byte read_byte;
String read_buffer = "";
String read_from_node = "";
int pinged[3] = {0, 0, 0};

unsigned long led_clock;

void setup() {
  Serial.begin(9600);
  HC12.begin(9600);
  
  FastLED.addLeds<WS2812, PIN, GRB>(leds, NUM_LEDS).setRgbw(RgbwDefault());
  FastLED.setBrightness(128);  // Set global brightness to 50%
  led_clock = millis();
}

void loop() {
  checkSerialInput();
  if (millis() - led_clock > 30) {
    updateLEDs();
    led_clock = millis();
  }
  //
  //delay(10);
}

void checkSerialInput() {
  while (HC12.available()) {
    read_byte = HC12.read();
    Serial.write(read_byte);
    if (read_byte == '\n') {
      //Serial.println(read_buffer);
      if (read_buffer[0] == 'P') {
        pinged[atoi(read_buffer[1])] = 255;
        //Serial.println((String) "Pinged by " + read_buffer[1]);
      }
      read_buffer = "";
    }
    else {
      read_buffer += (char) read_byte;
    }
  }

  while (Serial.available()) {
    read_from_node += Serial.read();
    if (read_from_node == "SC1") {
      leds[3] = CRGB(0, 255, 0);
      read_from_node = "";
    }
  }
}

void updateLEDs() {
  for (int i = 0; i < 3; i++) {
    if (pinged[i] > 0) {
      pinged[i] --;
      leds[i] = CRGB(0, pinged[i], 0);
      
    }
    if (pinged[i] == 0) {
      leds[i] = CRGB(255, 0, 0);
    }
  }
  FastLED.show();
}
