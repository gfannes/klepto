#include "esp_mac.h"
#include <Rover.hpp>
#include <WiFi.h>
#include <esp_now.h>

Rover rover;

const uint8_t senderMac[6] = {0x34, 0x85, 0x18, 0xAC, 0xf7, 0xa8};

typedef struct __attribute__((packed)) {
  int16_t joyX;
  int16_t joyY;
  uint8_t a;
  uint8_t b;
  uint8_t x;
  uint8_t y;
} BadgeInput;

// ----- Tune these for your joystick / motors -----
const int CENTER = 2048;     // joystick center (raw ADC)
const int DEADZONE = 200;    // ignore small wobbles around center
const int FULL_SCALE = 1800; // raw distance from center to "full"
const int PWM_STOP_L = 1550; // µs that makes the cont-rot servo stop
const int PWM_STOP_R = 1405; // µs that makes the cont-rot servo stop
const int PWM_RANGE = 500;   // full speed = STOP ± RANGE
// -------------------------------------------------
volatile BadgeInput lastPacket = {2048, 2048, 0, 0, 0, 0};
volatile bool newData = false;
unsigned long lastSeen = 0;
const unsigned long FAILSAFE_MS = 500;

bool macEquals(const uint8_t *a, const uint8_t *b) {
  for (int i = 0; i < 6; ++i)
    if (a[i] != b[i])
      return false;
  return true;
}

void handlePacket(const uint8_t *mac, const uint8_t *data, int len) {
  if (!macEquals(mac, senderMac))
    return;
  if (len != sizeof(BadgeInput))
    return;
  memcpy((void *)&lastPacket, data, sizeof(BadgeInput));
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
  if (abs(offset) < DEADZONE)
    return 0.0f;
  offset = (offset > 0) ? (offset - DEADZONE) : (offset + DEADZONE);
  float n = (float)offset / (float)(FULL_SCALE - DEADZONE);
  if (n > 1.0f)
    n = 1.0f;
  if (n < -1.0f)
    n = -1.0f;
  return n;
}

void setup() {
  Serial.begin(115200);
  delay(100);
  rover.setup();
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setSleep(false);
  Serial.print("Sender MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onRecv);
}

void loop() {
  unsigned long now = millis();
  if (newData) {
    lastSeen = now;
    newData = false;
  }
  bool linkOK = (now - lastSeen <= FAILSAFE_MS);

  // Snapshot the packet
  BadgeInput pkt;
  noInterrupts();
  memcpy(&pkt, (const void *)&lastPacket, sizeof(pkt));
  interrupts();

  if (!linkOK) {
    rover.stop();
    return;
  }

  const bool shoot = pkt.a;

  if (shoot) {
    Serial.println("shoot");
    rover.drive(0, 0);
    rover.shoot();
  } else {
    const auto forward = normalize(pkt.joyX);
    const auto steer = normalize(pkt.joyY);

    float left = forward + steer;
    float right = forward - steer;
    if (left > 1.0f)
      left = 1.0f;
    if (left < -1.0f)
      left = -1.0f;
    if (right > 1.0f)
      right = 1.0f;
    if (right < -1.0f)
      right = -1.0f;

    // left = -left;
    right = -right;

    if (false) {
      Serial.print(left);
      Serial.print(' ');
      Serial.print(right);
      Serial.print(' ');
      Serial.println("");
    }
    
    const int max_speed = 200;
    rover.drive(left * max_speed, right * max_speed);
  }
}
