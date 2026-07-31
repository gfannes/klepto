#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "esp_mac.h"
#include <ESP32Servo.h>

#define USER

const uint8_t senderMac[6] = {0x34, 0x85, 0x18, 0xAC, 0x37, 0xB8};

typedef struct __attribute__((packed)) {
  int16_t joyX;
  int16_t joyY;
  uint8_t a;
  uint8_t b;
  uint8_t x;
  uint8_t y;
} BadgeInput;

const int LEFT_SERVO_PIN  = 13;
const int RIGHT_SERVO_PIN = 14;

// ----- Tune these for your joystick / motors -----
const int CENTER       = 2048;   // joystick center (raw ADC)
const int DEADZONE     = 200;    // ignore small wobbles around center
const int FULL_SCALE   = 1800;   // raw distance from center to "full"
const int PWM_STOP_L     = 1550;   // µs that makes the cont-rot servo stop
const int PWM_STOP_R     = 1405;   // µs that makes the cont-rot servo stop
const int PWM_RANGE    = 500;    // full speed = STOP ± RANGE
const bool INVERT_LEFT  = false;
const bool INVERT_RIGHT = false;  // typical: right motor mounted mirrored
// -------------------------------------------------

Servo servoLeft, servoRight;

volatile BadgeInput lastPacket = {2048, 2048, 0, 0, 0, 0};
volatile bool newData = false;
unsigned long lastSeen = 0;
const unsigned long FAILSAFE_MS = 500;

bool macEquals(const uint8_t *a, const uint8_t *b) {
  for (int i = 0; i < 6; ++i) if (a[i] != b[i]) return false;
  return true;
}

void handlePacket(const uint8_t *mac, const uint8_t *data, int len) {
  if (!macEquals(mac, senderMac)) return;
  if (len != sizeof(BadgeInput)) return;
  memcpy((void*)&lastPacket, data, sizeof(BadgeInput));
  newData = true;
}

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
void onRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  handlePacket(info->src_addr, data, len);
}
#else
void onRecv(const uint8_t *mac, const uint8_t *data, int len) {
  handlePacket(mac, data, len);
}
#endif

float normalize(int raw) {
  int offset = raw - CENTER;
  if (abs(offset) < DEADZONE) return 0.0f;
  offset = (offset > 0) ? (offset - DEADZONE) : (offset + DEADZONE);
  float n = (float)offset / (float)(FULL_SCALE - DEADZONE);
  if (n >  1.0f) n =  1.0f;
  if (n < -1.0f) n = -1.0f;
  return n;
}

#define MY_SERIAL Serial

void setup() {
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);

  MY_SERIAL.begin(115200);
  MY_SERIAL.println();
  MY_SERIAL.println("Differential-drive receiver");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setSleep(false);

  

  if (esp_now_init() != ESP_OK) {
    MY_SERIAL.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onRecv);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  servoLeft.setPeriodHertz(50);
  servoRight.setPeriodHertz(50);
  servoLeft.attach(LEFT_SERVO_PIN, 1000, 2000);
  servoRight.attach(RIGHT_SERVO_PIN, 1000, 2000);
  servoLeft.writeMicroseconds(PWM_STOP_L);
  servoRight.writeMicroseconds(PWM_STOP_R);

  MY_SERIAL.println("Ready");
}

void loop() {

  // uint8_t mac[6];
  // esp_read_mac(mac, ESP_MAC_WIFI_STA);
  // MY_SERIAL.printf("Receiver MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
  //               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                
  unsigned long now = millis();
  if (newData) { lastSeen = now; newData = false; }
  bool linkOK = (now - lastSeen <= FAILSAFE_MS);

  // Snapshot the packet
  BadgeInput pkt;
  noInterrupts();
  memcpy(&pkt, (const void*)&lastPacket, sizeof(pkt));
  interrupts();

  // Dead-man's switch: must have link AND A held down
  bool armed = linkOK && pkt.a;

  int pulseLeft  = PWM_STOP_L;
  int pulseRight = PWM_STOP_R;
  float forward = 0, steer = 0;

  if (armed) {
    forward = normalize(pkt.joyX);
    steer   = normalize(pkt.joyY);

    float left  = forward - steer;
    float right = forward + steer;
    if (left  >  1.0f) left  =  1.0f;  if (left  < -1.0f) left  = -1.0f;
    if (right >  1.0f) right =  1.0f;  if (right < -1.0f) right = -1.0f;

    if (INVERT_LEFT)  left  = -left;
    if (INVERT_RIGHT) right = -right;

    pulseLeft  = PWM_STOP_L + (int)(left  * PWM_RANGE);
    pulseRight = PWM_STOP_R + (int)(right * PWM_RANGE);
  }

  servoLeft.writeMicroseconds(pulseLeft);
  servoRight.writeMicroseconds(pulseRight);

  static unsigned long lastPrint = 0;
  if (now - lastPrint >= 200) {
    lastPrint = now;
    const char* state =
        !linkOK ? "[FAILSAFE - no packets]" :
        !pkt.a  ? "[DISARMED - hold A]" :
                  "[ARMED]";
    MY_SERIAL.printf("fwd=%+.2f steer=%+.2f  L=%d µs  R=%d µs %s\n",
                  forward, steer, pulseLeft, pulseRight, state);
  }

  delay(20);
}
