#include "esp_mac.h"
#include <Rover.hpp>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

Rover rover;

#define BADGE_TEAM 1
const int ESPNOW_CHANNEL = 1;
const bool PAIR_FIRST_BADGE = true;

#if BADGE_TEAM == 0
// 34:85:18:AB:CF:38
const uint8_t badge_mac[6] = {0x34, 0x85, 0x18, 0xAB, 0xCF, 0x38};
#elif BADGE_TEAM == 1
// 90:70:69:00:88:38
const uint8_t badge_mac[6] = {0x90, 0x70, 0x69, 0x00, 0x88, 0x38};
#endif

typedef struct __attribute__((packed)) {
  int16_t joyX;
  int16_t joyY;
  uint8_t a;
  uint8_t b;
  uint8_t x;
  uint8_t y;
} BadgeInput;

const int CENTER = 2048;
const int DEADZONE = 200;
const int FULL_SCALE = 2048; 
// -------------------------------------------------
volatile BadgeInput received_packet = {2048, 2048, 0, 0, 0, 0};
BadgeInput prevPacket = {2048, 2048, 0, 0, 0, 0};
volatile bool newData = false;
unsigned long lastSeen = 0;
const unsigned long FAILSAFE_MS = 500;
uint8_t paired_badge_mac[6] = {};
bool hasPairedBadge = false;

bool macEquals(const uint8_t *a, const uint8_t *b) {
  for (int i = 0; i < 6; ++i)
    if (a[i] != b[i])
      return false;
  return true;
}

void printMac(const char *label, const uint8_t *mac) {
  Serial.printf("%s%02X:%02X:%02X:%02X:%02X:%02X",
                label, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void handlePacket(const uint8_t *mac, const uint8_t *data, int len) {

  if (len != sizeof(BadgeInput)) {
    printMac("Ignoring packet with wrong size from ", mac);
    Serial.printf(": got %d bytes, expected %u\n", len, (unsigned)sizeof(BadgeInput));
    return;
  }

  if (!hasPairedBadge && PAIR_FIRST_BADGE) {
    memcpy(paired_badge_mac, mac, sizeof(paired_badge_mac));
    hasPairedBadge = true;
    printMac("Paired badge ", paired_badge_mac);
    Serial.println();
  }

  if (!hasPairedBadge || !macEquals(mac, paired_badge_mac)) {
    printMac("Ignoring packet from ", mac);
    printMac(", expected ", paired_badge_mac);
    Serial.println();
    return;
  }

  memcpy((void *)&received_packet, data, sizeof(BadgeInput));
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
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setSleep(false);
  esp_err_t channelResult = esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  uint8_t macRaw[6];
  esp_read_mac(macRaw, ESP_MAC_WIFI_STA);
  printMac("Rover MAC: ", macRaw);
  Serial.println();
  if (PAIR_FIRST_BADGE) {
    Serial.println("Badge pairing: first valid 8-byte ESP-NOW packet wins");
  } else {
    memcpy(paired_badge_mac, badge_mac, sizeof(paired_badge_mac));
    hasPairedBadge = true;
    printMac("Badge MAC: ", paired_badge_mac);
    Serial.println();
  }
  Serial.printf("ESP-NOW channel: %d (%s)\n",
                ESPNOW_CHANNEL, channelResult == ESP_OK ? "ok" : "set failed");
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onRecv);

  Serial.print("Serial will be connection going down, rover uses serial pins for motor ...\n");
  delay(100);
  rover.setup();
  delay(100);

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
  memcpy(&pkt, (const void *)&received_packet, sizeof(pkt));
  interrupts();

  if (!linkOK) {
    rover.stop();
    return;
  }

  // if (pkt.a != prevPacket.a || pkt.b != prevPacket.b || pkt.x != prevPacket.x || pkt.y != prevPacket.y || pkt.joyX != prevPacket.joyX || pkt.joyY != prevPacket.joyY)
  // {
  //  Serial.printf("a: %d, b: %d, x: %d, y:%d, joyX:%d, joyY:%d\n", pkt.a, pkt.b, pkt.x, pkt.y, pkt.joyX, pkt.joyY);
  //  memcpy(&prevPacket, (const void *)&pkt, sizeof(pkt));
  //}


  unsigned int to_shoot = 0;
  if (pkt.a)
    to_shoot = 1;
  if(pkt.y)
    to_shoot = 3;

  if (to_shoot) {
    Serial.println("shoot");
    rover.drive(0, 0);
    rover.shoot(to_shoot);
  } else {

    if (pkt.x)
      rover.tilt_up();
    if (pkt.b)
      rover.tilt_down();

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
    
    rover.drive(left, right);
  }
}
