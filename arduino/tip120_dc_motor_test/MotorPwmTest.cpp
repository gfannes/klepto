/*
  130 DC motor PWM test using TIP120 low-side switch

  Board: Arduino Mega
  PWM signal: Arduino pin 9 -> base resistor -> TIP120 base
  Suggested base resistor: 1k
  Optional base pulldown: 10k from TIP120 base to ground

  Power wiring:
  - Battery +6V -> motor +
  - Motor - -> TIP120 collector
  - TIP120 emitter -> battery ground
  - Arduino GND -> battery ground

  Protection/noise parts:
  - 1N4006 flyback diode across the motor.
    Stripe/cathode goes to motor + / battery +6V.
    Unstriped/anode goes to motor - / TIP120 collector.
  - Optional 100 nF ceramic capacitor directly across motor terminals.

  Notes:
  - Do not power the motor from the Arduino 5V pin.
  - This circuit controls speed in one direction only.
  - The TIP120 drops noticeable voltage, so the motor will see less than the
    battery voltage when running.
*/

#include "MotorPwmTest.h"

#include <Arduino.h>

const byte MOTOR_PWM_PIN = 9;

const unsigned long STARTUP_DELAY_MS = 2000;
const unsigned long STEP_HOLD_MS = 1200;
const unsigned long STOP_HOLD_MS = 2000;

const int PWM_MIN = 0;
const int PWM_MAX = 255;
const int PWM_STEP = 15;

void setMotorPwm(int pwm) {
  pwm = constrain(pwm, PWM_MIN, PWM_MAX);

  Serial.print("PWM=");
  Serial.print(pwm);
  Serial.print(" duty=");
  Serial.print((pwm * 100L) / PWM_MAX);
  Serial.println("%");

  analogWrite(MOTOR_PWM_PIN, pwm);
}

void holdMotorPwm(int pwm, unsigned long durationMs) {
  setMotorPwm(pwm);
  delay(durationMs);
}

void setupMotorPwmTest() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for Serial Monitor on boards that need it.
  }

  pinMode(MOTOR_PWM_PIN, OUTPUT);
  analogWrite(MOTOR_PWM_PIN, 0);

  Serial.println("TIP120 130 DC motor PWM test");
  Serial.println("Motor will ramp from stopped to full speed and back down.");
  Serial.println("Watch for the lowest PWM value where the motor starts reliably.");

  delay(STARTUP_DELAY_MS);
}

void runMotorPwmTest() {
  holdMotorPwm(0, STOP_HOLD_MS);

  for (int pwm = PWM_STEP; pwm <= PWM_MAX; pwm += PWM_STEP) {
    holdMotorPwm(pwm, STEP_HOLD_MS);
  }

  holdMotorPwm(PWM_MAX, STEP_HOLD_MS);

  for (int pwm = PWM_MAX - PWM_STEP; pwm >= PWM_STEP; pwm -= PWM_STEP) {
    holdMotorPwm(pwm, STEP_HOLD_MS);
  }

  holdMotorPwm(0, STOP_HOLD_MS);
}
