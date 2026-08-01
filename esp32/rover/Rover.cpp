#include <Rover.hpp>

void Rover::setup() {
  Serial.println("Rover.setup()");

  pinMode(flywheel_pin, OUTPUT);

  // Typical hobby-servo pulse range. Adjust if required by your servos.
  servo_tilt.setPeriodHertz(50);
  servo_shoot.setPeriodHertz(50);
  const bool okk1 = servo_tilt.attach(servo_tilt_pin, 500, 2400);
  const bool okk2 = servo_shoot.attach(servo_shoot_pin, 500, 2400);
  Serial.printf("Servo attach: %d %d\n", okk1, okk2);
  servo_tilt.write(0);
  servo_shoot.write(0);

  static const int pwm_freq = 20000; // 20kHz, above audible range
  static const int pwm_res = 8;      // 0-255 duty

  const bool ok1 = ledcAttach(motor_a_in1, pwm_freq, pwm_res);
  const bool ok2 = ledcAttach(motor_a_in2, pwm_freq, pwm_res);
  const bool ok3 = ledcAttach(motor_b_in1, pwm_freq, pwm_res);
  const bool ok4 = ledcAttach(motor_b_in2, pwm_freq, pwm_res);
  Serial.printf("Motor ledcAttach: %d %d %d %d\n", ok1, ok2, ok3, ok4);  

  stop();
}

void Rover::shoot() {
  digitalWrite(flywheel_pin, HIGH);
  delay(1000);
  servo_shoot.attach(servo_shoot_pin, 500, 2400);
  servo_shoot.write(70);
  servo_tilt.attach(servo_tilt_pin, 500, 2400);
  servo_tilt.write(20);
  delay(500);
  servo_shoot.write(0);
  servo_tilt.write(0);
  digitalWrite(flywheel_pin, LOW);
  delay(500);
  servo_shoot.detach();
  servo_tilt.detach();
}

void Rover::drive(int l, int r) {
  drive_(motor_a_in1, motor_a_in2, l);
  drive_(motor_b_in1, motor_b_in2, r);
}

void Rover::drive_(int in1, int in2, int speed) {
  speed = constrain(speed, -255, 255);
  if (speed >= 0) {
    ledcWrite(in1, 0);
    ledcWrite(in2, speed);
  } else {
    ledcWrite(in1, -speed);
    ledcWrite(in2, 0);
  }
}

void Rover::stop() {
  digitalWrite(flywheel_pin, LOW);

  ledcWrite(motor_a_in1, 0);
  ledcWrite(motor_a_in2, 0);
  ledcWrite(motor_b_in1, 0);
  ledcWrite(motor_b_in2, 0);
}
