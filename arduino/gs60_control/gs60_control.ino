#include <ESP32Servo.h>

Servo gs60;
const int servoPin = 27;

void setup() {
  // ESP32Servo needs timers allocated for PWM generation
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  gs60.setPeriodHertz(50);            // standard 50 Hz servo signal
  gs60.attach(servoPin, 500, 2400);  // pin, min pulse (µs), max pulse (µs)
}

void loop() {
  // Sweep 0 -> 180 degrees
  // gs60.write(90);
  // delay(10000);
  // gs60.detach();

  int min_angle = 120;
  int max_angle = 170;

  for (int angle = min_angle; angle <= max_angle; angle++) {
    gs60.write(angle);
    delay(15);
  }
  // Sweep 180 -> 0 degrees
  for (int angle = max_angle; angle >= min_angle; angle--) {
    gs60.write(angle);
    delay(15);
  }
}