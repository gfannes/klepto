#include "esp_mac.h"
#include <Rover.hpp>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

Rover rover;

const int ESPNOW_CHANNEL = 1;
const uint8_t ROVER_ID = 'A';

typedef struct __attribute__((packed)) {
  int16_t joyX;
  int16_t joyY;
  uint8_t a;
  uint8_t b;
  uint8_t x;
  uint8_t y;
  uint8_t roverId;
} BadgeInput;

const int CENTER = 2048;
const int DEADZONE = 200;
const int FULL_SCALE = 2048; 
// -------------------------------------------------
volatile BadgeInput received_packet = {2048, 2048, 0, 0, 0, 0, ROVER_ID};
BadgeInput prevPacket = {2048, 2048, 0, 0, 0, 0, ROVER_ID};
volatile bool newData = false;
unsigned long lastSeen = 0;
const unsigned long FAILSAFE_MS = 500;

void printMac(const char *label, const uint8_t *mac) {
  Serial.printf("%s%02X:%02X:%02X:%02X:%02X:%02X",
                label, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void handlePacket(const uint8_t *mac, const uint8_t *data, int len) {
  (void)mac;
  if (len != sizeof(BadgeInput)) {
    return;
  }

  BadgeInput packet;
  memcpy(&packet, data, sizeof(packet));
  if (packet.roverId != ROVER_ID) {
    return;
  }

  memcpy((void *)&received_packet, &packet, sizeof(BadgeInput));
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
  Serial.printf("Rover ID: %c\n", ROVER_ID);
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

  if (pkt.roverId != prevPacket.roverId || pkt.a != prevPacket.a || pkt.b != prevPacket.b ||
      pkt.x != prevPacket.x || pkt.y != prevPacket.y || pkt.joyX != prevPacket.joyX ||
      pkt.joyY != prevPacket.joyY)
  {
    Serial.printf("joyX:%d joyY:%d a:%u b:%u x:%u y:%u id:%c idByte:%u\n",
                  pkt.joyX, pkt.joyY, pkt.a, pkt.b, pkt.x, pkt.y,
                  (char)pkt.roverId, pkt.roverId);
    memcpy(&prevPacket, (const void *)&pkt, sizeof(pkt));
  }


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
