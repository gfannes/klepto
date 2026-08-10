// Library Dependency: bundled Fri3dBadge2024 library
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "Fri3dBadge_pins.h"
#include "Fri3dBadge_Button.h"

// Receiver's MAC: 08:D1:F9:CB:F9:CC
// B0:CB:D8:89:75:E8

uint8_t peerAddress[] = {0xB0, 0xCB, 0xD8, 0x89, 0x75, 0xE8};

typedef struct __attribute__((packed)) {
  int16_t joyX;
  int16_t joyY;
  uint8_t a;
  uint8_t b;
  uint8_t x;
  uint8_t y;
} BadgeInput;

BadgeInput pkt;

enum { BTN_A, BTN_B, BTN_X, BTN_Y, NUM_BTN };
Fri3d_Button *buttons[NUM_BTN] = {
  new Fri3d_Button(FRI3D_BUTTON_TYPE_DIGITAL, PIN_A, 25, INPUT_PULLUP, true),
  new Fri3d_Button(FRI3D_BUTTON_TYPE_DIGITAL, PIN_B, 25, INPUT_PULLUP, true),
  new Fri3d_Button(FRI3D_BUTTON_TYPE_DIGITAL, PIN_X, 25, INPUT_PULLUP, true),
  new Fri3d_Button(FRI3D_BUTTON_TYPE_DIGITAL, PIN_Y, 25, INPUT_PULLUP, true),
};

const int JOY_THRESHOLD = 100;
int lastJoyX = -9999, lastJoyY = -9999;

void onSent(const uint8_t *mac, esp_now_send_status_t status) {
  // With unicast, this tells you whether the receiver ACKed.
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("  (send failed - no ACK)");
  }
}

void sendPacket() {
  esp_err_t r = esp_now_send(peerAddress, (uint8_t*)&pkt, sizeof(pkt));
  // if (r != ESP_OK) {
    // Serial.printf("esp_now_send returned %d\n", r);
    // return;
  // }
  // Serial.printf("TX X=%d Y=%d A=%d B=%d Xb=%d Yb=%d\n",
                // pkt.joyX, pkt.joyY, pkt.a, pkt.b, pkt.x, pkt.y);
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Fri3d Badge sender -> 08:D1:F9:CB:F9:CC");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
   WiFi.setSleep(false);   // <-- add this line
  Serial.print("Sender MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, peerAddress, 6);
  peer.channel = 0;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  for (int i = 0; i < NUM_BTN; ++i) buttons[i]->begin();
  pinMode(PIN_JOY_X, INPUT);
  pinMode(PIN_JOY_Y, INPUT);
}

const unsigned long SEND_INTERVAL_MS = 50;   // 20 Hz
unsigned long lastSend = 0;

void loop() {
  // Poll buttons every iteration so debouncing works
  for (int i = 0; i < NUM_BTN; ++i) buttons[i]->read();

  unsigned long now = millis();
  if (now - lastSend < SEND_INTERVAL_MS) return;
  lastSend = now;

  pkt.a = buttons[BTN_A]->isPressed() ? 1 : 0;
  pkt.b = buttons[BTN_B]->isPressed() ? 1 : 0;
  pkt.x = buttons[BTN_X]->isPressed() ? 1 : 0;
  pkt.y = buttons[BTN_Y]->isPressed() ? 1 : 0;
  pkt.joyX = analogRead(PIN_JOY_X);
  pkt.joyY = analogRead(PIN_JOY_Y);

  esp_err_t r = esp_now_send(peerAddress, (uint8_t*)&pkt, sizeof(pkt));
}
