#include <ESP32Servo.h>

// Connected devices
constexpr uint8_t DC130_PIN = 25;

constexpr uint8_t T1_A_IN1 = 22;
constexpr uint8_t T1_A_IN2 = 23;
constexpr uint8_t T1_B_IN1 = 18;
constexpr uint8_t T1_B_IN2 = 19;

constexpr uint8_t SERVO_1_PIN = 27;
constexpr uint8_t SERVO_2_PIN = 14;

constexpr unsigned long MOTOR_TEST_MS = 2000;
constexpr unsigned long PAUSE_MS = 750;

Servo servo1;
Servo servo2;

void stopAllMotors()
{
    digitalWrite(DC130_PIN, LOW);

    digitalWrite(T1_A_IN1, LOW);
    digitalWrite(T1_A_IN2, LOW);
    digitalWrite(T1_B_IN1, LOW);
    digitalWrite(T1_B_IN2, LOW);
}

void testDC130Motor()
{
    Serial.println("DC130 motor: ON");
    // analogWrite(DC130_PIN, 255);
    digitalWrite(DC130_PIN, HIGH);
    delay(MOTOR_TEST_MS);
    // analogWrite(DC130_PIN, 0);
    digitalWrite(DC130_PIN, LOW);
    Serial.println("DC130 motor: OFF");
    delay(PAUSE_MS);
}

void driveT1(uint8_t in1, uint8_t in2, int direction)
{
    if (direction > 0)
    {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
    }
    else if (direction < 0)
    {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
    }
    else
    {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
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

void testServo(Servo &servo, const char *name)
{
    Serial.printf("%s: 45 degrees\n", name);
    servo.write(45);
    delay(1000);

    Serial.printf("%s: 90 degrees\n", name);
    servo.write(90);
    delay(1000);

    Serial.printf("%s: 135 degrees\n", name);
    servo.write(135);
    delay(1000);

    Serial.printf("%s: return to 90 degrees\n", name);
    servo.write(90);
    delay(PAUSE_MS);
}

void setup()
{
    Serial.begin(115200);

    pinMode(DC130_PIN, OUTPUT);
    pinMode(T1_A_IN1, OUTPUT);
    pinMode(T1_A_IN2, OUTPUT);
    pinMode(T1_B_IN1, OUTPUT);
    pinMode(T1_B_IN2, OUTPUT);
    stopAllMotors();

    // Typical hobby-servo pulse range. Adjust if required by your servos.
    servo1.setPeriodHertz(50);
    servo2.setPeriodHertz(50);
    servo1.attach(SERVO_1_PIN, 500, 2400);
    servo2.attach(SERVO_2_PIN, 500, 2400);
    servo1.write(90);
    servo2.write(90);

    Serial.println("\nESP32 device test starts in 3 seconds...");
    delay(3000);
}

void loop()
{
    Serial.println("\n--- Starting test cycle ---");

    testDC130Motor();

    testT1Motor("T1 motor A", T1_A_IN1, T1_A_IN2);
    testT1Motor("T1 motor B", T1_B_IN1, T1_B_IN2);

    testServo(servo1, "Servo 1");
    testServo(servo2, "Servo 2");

    stopAllMotors();
    Serial.println("--- Test cycle complete; repeating in 2 seconds ---");
    delay(2000);
}
