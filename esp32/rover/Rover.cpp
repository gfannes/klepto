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
  tilt_degrees = tilt_degree_down;
  servo_tilt.write(tilt_degrees);
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



void Rover::shoot(unsigned int shoot) {
  digitalWrite(flywheel_pin, HIGH);
  servo_shoot.attach(servo_shoot_pin, 500, 2400);
  delay(1000);

  for(unsigned int i = 0; i < shoot; ++i)
  {
    servo_shoot.write(70);
    delay(100);
    servo_shoot.write(0);
    delay(300);
  }

  digitalWrite(flywheel_pin, LOW);
  delay(250);
  servo_shoot.detach();
}




void Rover::tilt_up()
{
  if (tilt_degree_up > tilt_degree_down)
    tilt_degrees = constrain(tilt_degrees + tilt_degrees_per_update, tilt_degree_down, tilt_degree_up);
  else
    tilt_degrees = constrain(tilt_degrees - tilt_degrees_per_update, tilt_degree_up, tilt_degree_down);
  servo_tilt.write(tilt_degrees);
}
  
void Rover::tilt_down()
{
  if (tilt_degree_up > tilt_degree_down)
    tilt_degrees = constrain(tilt_degrees - tilt_degrees_per_update, tilt_degree_down, tilt_degree_up);
  else
    tilt_degrees = constrain(tilt_degrees + tilt_degrees_per_update, tilt_degree_up, tilt_degree_down);
  servo_tilt.write(tilt_degrees);
}

void Rover::drive(float l, float r) {

  drive_(motor_a_in1, motor_a_in2, l);
  drive_(motor_b_in1, motor_b_in2, r);
}



void Rover::drive_(int in1, int in2, float speed) {


  const float dead_zone = 0.1;
  if (fabs(speed) < dead_zone)
  {
    ledcWrite(in1, 0);
    ledcWrite(in2, 0);
  }
  else
  {
    
    // we remap so that 0 -> min_pwm_value and 1 -> max_pwm_value
    const int min_pwm_value = 150;
    const int max_pwm_value = 255;

    if (speed >= 0)
    {
      int mapped_value = constrain(min_pwm_value + speed * (max_pwm_value - min_pwm_value), -max_pwm_value, max_pwm_value);
      ledcWrite(in1, 0);
      ledcWrite(in2, mapped_value);
    }
    else
    {
      int mapped_value = constrain(min_pwm_value + -speed * (max_pwm_value - min_pwm_value), -max_pwm_value, max_pwm_value);
      ledcWrite(in1, mapped_value);
      ledcWrite(in2, 0);

    }
  }
}

void Rover::stop() {
  digitalWrite(flywheel_pin, LOW);
  
  tilt_degrees = tilt_degree_down;
  servo_tilt.write(tilt_degrees);

  ledcWrite(motor_a_in1, 0);
  ledcWrite(motor_a_in2, 0);
  ledcWrite(motor_b_in1, 0);
  ledcWrite(motor_b_in2, 0);
}
