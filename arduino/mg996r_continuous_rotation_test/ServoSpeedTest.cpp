/*
  MG996R continuous-rotation servo speed test

  Board: Arduino Mega
  Signal: servo signal wire to pin 9
  Ground: servo ground and Arduino ground must be connected together

  Power notes:
  - Do not power an MG996R drive servo from the Arduino 5V pin or onboard
    regulator. The servo can draw far more current than the Arduino can safely
    supply, especially at startup, stall, or when driving a wheel.
  - A 5-cell NiMH pack is a reasonable direct servo supply for many MG996R
    variants: about 6.0 V nominal and roughly 7.0 V hot off the charger.
    Check your exact servo's voltage rating before using it directly.
  - A buck converter is useful if you want a fixed servo rail, for example
    5.5-6.0 V. Choose one rated for the expected stall/startup current of both
    servos, not just their no-load current.
  - Power the Arduino separately from USB or its barrel/Vin input while testing,
    unless you have a deliberately designed shared power system.

  Continuous-rotation behavior:
  - 1500 us is the usual stop/neutral command.
  - Below neutral rotates one direction, above neutral rotates the other.
  - Converted servos often need neutral calibration. If the servo creeps during
    STOP, adjust STOP_US until it stays still.
  - If forward/backward are reversed for your mounting, swap the signs of the
    offsets in the four *_US constants below.
*/

#include "ServoSpeedTest.h"

#include <Arduino.h>
#include <Servo.h>

const byte SERVO_PIN = 9;

// Tune these for your converted servo.
const int STOP_US = 1500;
const int SLOW_OFFSET_US = 80;
const int FULL_OFFSET_US = 500;

const int FULL_FWD_US = STOP_US + FULL_OFFSET_US;
const int SLOW_FWD_US = STOP_US + SLOW_OFFSET_US;
const int SLOW_BWD_US = STOP_US - SLOW_OFFSET_US;
const int FULL_BWD_US = STOP_US - FULL_OFFSET_US;

const unsigned long FULL_STEP_MS = 3000;
const unsigned long SLOW_STEP_MS = 3000;
const unsigned long STOP_STEP_MS = 2000;

Servo driveServo;

struct Step {
  const char *label;
  int pulseUs;
  unsigned long durationMs;
};

const Step cycle[] = {
    {"full-speed forward", FULL_FWD_US, FULL_STEP_MS},
    {"slow-speed forward", SLOW_FWD_US, SLOW_STEP_MS},
    {"stop", STOP_US, STOP_STEP_MS},
    {"slow-speed backward", SLOW_BWD_US, SLOW_STEP_MS},
    {"full-speed backward", FULL_BWD_US, FULL_STEP_MS},
    {"slow-speed backward", SLOW_BWD_US, SLOW_STEP_MS},
    {"stop", STOP_US, STOP_STEP_MS},
    {"slow-speed forward", SLOW_FWD_US, SLOW_STEP_MS},
};

const size_t STEP_COUNT = sizeof(cycle) / sizeof(cycle[0]);

void commandServo(const Step &step) {
  Serial.print(step.label);
  Serial.print("  pulse=");
  Serial.print(step.pulseUs);
  Serial.println(" us");

  driveServo.writeMicroseconds(step.pulseUs);
  delay(step.durationMs);
}

void setupServoSpeedTest() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for Serial Monitor on boards that need it.
  }

  driveServo.attach(SERVO_PIN, 1000, 2000);
  driveServo.writeMicroseconds(STOP_US);

  Serial.println("MG996R continuous-rotation speed test");
  Serial.println("Starting in neutral. Adjust STOP_US if the servo creeps.");
  delay(3000);
}

void runServoSpeedTest() {
  // for (size_t i = 0; i < STEP_COUNT; ++i) {
  //   commandServo(cycle[i]);
  // }
  const size_t delay_ms = 300;
  const size_t step = 20;
  for (size_t pulse_us = FULL_BWD_US; pulse_us <= FULL_FWD_US; pulse_us += step) {
    driveServo.writeMicroseconds(pulse_us);
    delay(delay_ms);
  }
  for (size_t pulse_us = FULL_FWD_US; pulse_us <= FULL_BWD_US; pulse_us += step) {
    driveServo.writeMicroseconds(pulse_us);
    delay(delay_ms);
  }
}
