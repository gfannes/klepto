#include <Rover.hpp>

void Rover::setup() {
  Serial.println("Rover.setup()");

  pinMode(flywheel_pin, OUTPUT);

  pinMode(motor_a_in1, OUTPUT);
  pinMode(motor_a_in2, OUTPUT);
  pinMode(motor_b_in1, OUTPUT);
  pinMode(motor_b_in2, OUTPUT);

  stop();

  // Typical hobby-servo pulse range. Adjust if required by your servos.
  servo_tilt.setPeriodHertz(50);
  servo_shoot.setPeriodHertz(50);
  servo_tilt.attach(servo_tilt_pin, 500, 2400);
  servo_shoot.attach(servo_shoot_pin, 500, 2400);
  servo_tilt.write(0);
  servo_shoot.write(0);
}

void Rover::shoot() {
  digitalWrite(flywheel_pin, HIGH);
  delay(1000);
  servo_shoot.attach(servo_shoot_pin, 500, 2400);
  servo_shoot.write(70);
  delay(500);
  servo_shoot.write(0);
  digitalWrite(flywheel_pin, LOW);
  delay(500);
  servo_shoot.detach();
}

void Rover::drive(int l, int r) {
  drive_(motor_a_in1, motor_a_in2, l);
  drive_(motor_b_in1, motor_b_in2, r);
}

void Rover::drive_(int in1, int in2, int speed) {
  if (speed >= 0) {
    digitalWrite(in1, LOW);
    analogWrite(in2, speed);
  } else {
    analogWrite(in1, -speed);
    digitalWrite(in2, LOW);
  }
}

void Rover::stop() {
  digitalWrite(flywheel_pin, LOW);

  digitalWrite(motor_a_in1, LOW);
  digitalWrite(motor_a_in2, LOW);
  digitalWrite(motor_b_in1, LOW);
  digitalWrite(motor_b_in2, LOW);
}
