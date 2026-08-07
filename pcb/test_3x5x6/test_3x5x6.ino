#include <ESP32Servo.h>

// Connected devices
constexpr uint8_t DC130_PIN = 33;

constexpr uint8_t T1_A_IN1 = 1;
constexpr uint8_t T1_A_IN2 = 3;
// constexpr uint8_t T1_A_IN1 = 22;
// constexpr uint8_t T1_A_IN2 = 23;
constexpr uint8_t T1_B_IN1 = 19;
constexpr uint8_t T1_B_IN2 = 18;

constexpr uint8_t SERVO_1_PIN = 5;
constexpr uint8_t SERVO_2_PIN = 17;

constexpr unsigned long MOTOR_TEST_MS = 2000;
constexpr unsigned long PAUSE_MS = 750;

Servo servo1;
Servo servo2;

void stopAllMotors()
{

    digitalWrite(DC130_PIN, LOW);

    servo1.write(0);
    servo2.write(0);

    ledcWrite(T1_A_IN1, 0);
    ledcWrite(T1_A_IN2, 0);
    ledcWrite(T1_B_IN1, 0);
    ledcWrite(T1_B_IN2, 0);
}

void testDC130Motor(bool on)
{
    if (on)
    {
        Serial.println("DC130 motor: ON");
        // analogWrite(DC130_PIN, 255);
        digitalWrite(DC130_PIN, HIGH);
        delay(MOTOR_TEST_MS);
    }
    else
    {
        // analogWrite(DC130_PIN, 0);
        digitalWrite(DC130_PIN, LOW);
        Serial.println("DC130 motor: OFF");
        delay(PAUSE_MS);
    }
}

void driveT1(uint8_t in1, uint8_t in2, int direction)
{
    if (direction > 0)
    {
        ledcWrite(in1, 255);
        ledcWrite(in2, 0);
    }
    else if (direction < 0)
    {
        ledcWrite(in1, 0);
        ledcWrite(in2, 255);
    }
    else
    {
        ledcWrite(in1, 0);
        ledcWrite(in2, 0);
    }
}

void testT1Motor(const char *name, uint8_t in1, uint8_t in2)
{
    Serial.printf("%s: forward\n", name);
    driveT1(in1, in2, 1);
    delay(MOTOR_TEST_MS);

    driveT1(in1, in2, 0);
    delay(PAUSE_MS);

    Serial.printf("%s: reverse\n", name);
    driveT1(in1, in2, -1);
    delay(MOTOR_TEST_MS);

    driveT1(in1, in2, 0);
    delay(PAUSE_MS);
}

void testServos(Servo &servo1, Servo &servo2)
{
    Serial.printf("45 degrees\n");
    servo1.write(45);
    servo2.write(45);
    delay(1000);

    Serial.printf("90 degrees\n");
    servo1.write(90);
    servo2.write(90);
    delay(1000);

    Serial.printf("135 degrees\n");
    servo1.write(135);
    servo2.write(135);
    delay(1000);

    Serial.printf("return to 90 degrees\n");
    servo1.write(90);
    servo2.write(90);
    delay(PAUSE_MS);
}

void setup()
{
    Serial.begin(115200);

    pinMode(DC130_PIN, OUTPUT);

    // Typical hobby-servo pulse range. Adjust if required by your servos.
    servo1.setPeriodHertz(50);
    servo2.setPeriodHertz(50);
    servo1.attach(SERVO_1_PIN, 500, 2400);
    servo2.attach(SERVO_2_PIN, 500, 2400);
    servo1.write(0);
    servo2.write(0);

    static const int pwm_freq = 20000; // 20kHz, above audible range
    static const int pwm_res = 8; // 0-255 duty

    const bool ok1 = ledcAttach(T1_A_IN1, pwm_freq, pwm_res);
    const bool ok2 = ledcAttach(T1_A_IN2, pwm_freq, pwm_res);
    const bool ok3 = ledcAttach(T1_B_IN1, pwm_freq, pwm_res);
    const bool ok4 = ledcAttach(T1_B_IN2, pwm_freq, pwm_res);
    stopAllMotors();

    Serial.println("\nESP32 device test starts in 3 seconds...");
    delay(3000);
}

void loop()
{
    Serial.println("\n--- Starting test cycle ---");

    testDC130Motor(true);

    testServos(servo1, servo2);

    testDC130Motor(false);

    testT1Motor("T1 motor A", T1_A_IN1, T1_A_IN2);
    testT1Motor("T1 motor B", T1_B_IN1, T1_B_IN2);

    stopAllMotors();
    Serial.println("--- Test cycle complete; repeating in 2 seconds ---");
    delay(2000);
}
