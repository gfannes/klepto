#ifndef HEADER_Rover_hpp_ALREADY_INCLUDED
#define HEADER_Rover_hpp_ALREADY_INCLUDED

#include <ESP32Servo.h>
#include "Arduino.h"

#ifndef ENABLE_ROVER_LOGGING
#define ENABLE_ROVER_LOGGING 0
#endif

const uint8_t ROVER_ID = 'A';

#if ENABLE_ROVER_LOGGING
#define ROVER_LOG_BEGIN(...) Serial.begin(__VA_ARGS__)
#define ROVER_LOG_PRINT(...) Serial.print(__VA_ARGS__)
#define ROVER_LOG_PRINTLN(...) Serial.println(__VA_ARGS__)
#define ROVER_LOG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define ROVER_LOG_BEGIN(...)
#define ROVER_LOG_PRINT(...)
#define ROVER_LOG_PRINTLN(...)
#define ROVER_LOG_PRINTF(...)
#endif

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
    const int motor_a_in1 = 1;
    const int motor_a_in2 = 3;
    const int motor_b_in1 = 19;
    const int motor_b_in2 = 18;
    const int servo_tilt_pin = 5;
    const int servo_shoot_pin = 17;
#else
    const int flywheel_pin = 33;
    const int motor_a_in1 = 22;
    const int motor_a_in2 = 23;
    const int motor_b_in1 = 19;
    const int motor_b_in2 = 18;
    const int servo_tilt_pin = 5;
    const int servo_shoot_pin = 17;
#endif
    const int tilt_degrees_per_update = 1;
    const int tilt_degree_down = 180;
    const int tilt_degree_up = 140;

    Servo servo_tilt;
    Servo servo_shoot;
    int tilt_degrees;

    void drive_(int in1, int in2, float speed);
};

#endif
