#ifndef HEADER_Rover_hpp_ALREADY_INCLUDED
#define HEADER_Rover_hpp_ALREADY_INCLUDED

#include <ESP32Servo.h>
#include "Arduino.h"

class Rover
{
public:
    void setup();
    void drive(float l, float r);
    void shoot(unsigned int count);
    void stop();
    void tilt_up();
    void tilt_down();

private:
#if 1
    const int flywheel_pin = 33;
    // const int motor_a_in1 = 1;
    // const int motor_a_in2 = 3;
    const int motor_a_in1 = 22;
    const int motor_a_in2 = 23;
    const int motor_b_in1 = 19;
    const int motor_b_in2 = 18;
    const int servo_tilt_pin = 5;
    const int servo_shoot_pin = 17;
#else
    const int flywheel_pin = 25;
    const int motor_a_in1 = 22;
    const int motor_a_in2 = 23;
    const int motor_b_in1 = 18;
    const int motor_b_in2 = 19;
    const int servo_tilt_pin = 27;
    const int servo_shoot_pin = 14;
#endif
    const int tilt_degrees_per_update = 1;

    Servo servo_tilt;
    Servo servo_shoot;
    int tilt_degrees;

    void drive_(int in1, int in2, float speed);
};

#endif
