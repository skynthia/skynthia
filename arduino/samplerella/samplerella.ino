#include <Wire.h>
#include "Adafruit_MPR121.h"

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

Adafruit_MPR121 mpr = Adafruit_MPR121();

uint16_t lasttouched = 0;
uint16_t currtouched = 0;
int touched_count = 0;
bool pause_drums = false;
int touch_calibration[3] = {0, 0, 0};

// hall effect
int right_tentacle_short[2] = {22, 23};
int rts_val = 0;
int right_tentacle_long[2] = {24, 25};
int rtl_val = 0;
int left_tentacle = 26;
int lt_val = 0;

int sample_num = 0;
int change_sample = 0;

int motor = 50;

// photoresistors
int eyes[2] = {A8, A9};
int eye_calibration[2] = {0, 0};

// timing
long eye_active[2] = {-1, -1};
long lt_active = -1;

long ping_clock;
long reset_clock = -1;

long haptics_clock;
int haptics_dur;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("Booting Samplerella...");
  
  pinMode(right_tentacle_short[0], INPUT);
  pinMode(right_tentacle_short[1], INPUT);
  pinMode(right_tentacle_long[0], INPUT);
  pinMode(right_tentacle_long[1], INPUT);
  pinMode(left_tentacle, INPUT);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, HIGH);

  if (!mpr.begin(0x5A)) {
    Serial.println("MPR121 not found");
    while (1);
  }
  
  calibrate();

  Serial.println("Finished booting Samplerella.");
  transmit("G1");
}

void loop() {
  //checkEyes();
  checkTentacles();
  checkTouch();
  checkHaptics();
  checkPing();
  delay(10);
}

void checkEyes() {
  // top eye: next/previous track
  // bottom eye: start/stop backing??
  for (int i = 0; i < 2; i++) {
    int eye_val = analogRead(eyes[i]);
    //Serial.println(eye_val);
    delay(100);
    if ((eye_val - eye_calibration[i]) >= 200 && eye_active[i] == -1) {
      // do a buzz here so I can tell if this is happening
      sendHaptics(250 + (250*i));
      
      // does Samp need eyelids???
      Serial.print("Activate eye ");
      Serial.println(i);
      eye_active[i] = millis();
    }
    else if ((eye_val - eye_calibration[i]) < 100 && eye_active[i] > -1) {
      Serial.print("Deactivate eye ");
      Serial.println(i);
      if (millis() - eye_active[i] > 1000) {
        sendHaptics(250 + (250*i));
        if (i == 0) {
          Serial.println("Next track");
          sample_num = 0;
          transmit("PNT");
        }
        else {
          Serial.print("Play/stop clip ");
          Serial.println(rts_val);
          transmit((String) "PC" + (char) (rts_val + 65));
          delay(100);
        }
      }
      eye_active[i] = -1;
    }
  }
}

void checkTentacles() {
  // right tentacle
  int rts_new = 0;
  int rtl_new = 0;
  for (int i = 0; i < 2; i++) {
    int rts = !digitalRead(right_tentacle_short[i]);
    //Serial.println(rts);
    int rtl = !digitalRead(right_tentacle_long[i]);
    rts_new = rts_new | (rts << i);
    rtl_new = rtl_new | (rtl << i);
  }
  if (rts_val != rts_new) {
    rts_val = rts_new;
    Serial.print("New RTS val: ");
    Serial.println(rts_val);
    sendHaptics(100 + (100*rts_val));
    // until I figure out wtf is wrong with the photoresistors (if ever)
    if (rts_val == 1) {
      Serial.println("Next track");
      sample_num = 0;
      transmit("PNT");
    }
    else if (rts_val == 2) {
      Serial.println("Play/stop clip");
      transmit("PCA");
    }
  }
  if (rtl_val != rtl_new) {
    rtl_val = rtl_new;
    Serial.print("Change vocal effects: ");
    Serial.println(rtl_val);
    transmit((String) "PV" + (char) (rtl_val + 65));
    delay(100);
    sendHaptics(200 + (200*rtl_val));
  }

  // left tentacle
  int lt_new = digitalRead(left_tentacle);
  if (lt_val != lt_new) {
    lt_val = lt_new;
    if (!lt_val && lt_active == -1) {
      lt_active = millis();
      sendHaptics(100);
    }
    else if (lt_active != -1) {
      if (change_sample == 0) {
        Serial.println("Send current sample");
        transmit((String) "PS" + (char) (sample_num + 65));
        sendHaptics(100);
        reset_clock = millis();
      }
      else {
        sample_num += change_sample;
        sample_num = max(sample_num, 0);
        Serial.print("Change sample by ");
        Serial.println(change_sample);
        sendHaptics(300 + (sample_num * 100));
        change_sample = 0;
      }
      lt_active = -1;
    }
  }

  if (lt_active != -1 && millis() - lt_active > 2000 && change_sample == 1) {
    change_sample = -1;
    sendHaptics(200);
    lt_active = millis();
  }
  else if (lt_active != -1 && millis() - lt_active > 1000 && change_sample == 0) {
    change_sample = 1;
    sendHaptics(200);
    lt_active = millis();
  }

  if (millis() - reset_clock > 1000) {
    reset_clock = -1;
  }
}

void checkTouch() {
  currtouched = mpr.touched();
  touched_count = 0;

  for (uint8_t i=0; i<3; i++) {
    uint16_t baseline = mpr.baselineData(i);
    /*if (reset_clock != -1 && millis() - reset_clock >= 200 && baseline < 500) {
      mpr.begin(0x5A);
      reset_clock = -1;
    }*/
    uint16_t diff = touch_calibration[i] - mpr.filteredData(i);
    //Serial.print((String) mpr.baselineData(i) + "\t");
    //Serial.print((String) touch_calibration[i] + "\t");
    //Serial.print((String) mpr.filteredData(i) + "\t");
    //Serial.print((String) diff + "\t");
    if (diff > 30 && diff < 1000 && lt_active == -1) {
      touched_count++;
      //Serial.println((String) "touched " + i);
    }
  }
  //Serial.println();
  
  if (touched_count >= 2 && !pause_drums) {
    Serial.println("Pause drums");
    transmit("PDP");
    pause_drums = true;
  }
  else if (touched_count == 0 && pause_drums) {
    Serial.println("Unpause drums");
    transmit("PDU");
    pause_drums = false;
  }

  lasttouched = currtouched;
}

void calibrate() {
  Serial.println("Calibrating...");
  
  //digitalWrite(motor, LOW);
  
  for (int i = 0; i < 10; i++) {
    eye_calibration[0] += analogRead(A8);
    eye_calibration[1] += analogRead(A9);

    touch_calibration[0] += mpr.filteredData(0);
    touch_calibration[1] += mpr.filteredData(1);
    touch_calibration[2] += mpr.filteredData(2);
    delay(100);
  }
  eye_calibration[0] /= 10;
  eye_calibration[1] /= 10;
  
  touch_calibration[0] = touch_calibration[0]/10 + 10;
  touch_calibration[1] = touch_calibration[1]/10 + 10;
  touch_calibration[2] = touch_calibration[2]/10 + 10;
  
  Serial.print("Finished calibrating photoresistors: ");
  Serial.print(eye_calibration[0]);
  Serial.print(", ");
  Serial.println(eye_calibration[1]);

  Serial.print("Finished calibrating touch sensor: ");
  Serial.print(touch_calibration[0]);
  Serial.print(", ");
  Serial.print(touch_calibration[1]);
  Serial.print(", ");
  Serial.println(touch_calibration[2]);

  if (eye_calibration[0] > 750 || eye_calibration[1] > 750) {
    Serial.println("TOO MUCH LIGHT");
    return;
  }
  
  //digitalWrite(motor, HIGH);
}

void transmit(String val) {
  Serial1.println(val);
  ping_clock = millis();
}

void checkPing() {
  if (millis() - ping_clock > 10000) {
    transmit("G1");
  }
}
