//this is slave 3
//K
//O

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

#define TRIG 32
#define ECHO 33

uint8_t HELTEC1[] = {0x24,0x58,0x7C,0x5B,0x3C,0xA8};
uint8_t NODE1[]   = {0x44,0x17,0x93,0xE4,0xDE,0x90};
uint8_t NODE2[]   = {0x68,0xFE,0x71,0x03,0x38,0x1C};

typedef struct {
  int node_id;
  float soil_moisture;
  float soil_temperature;
  float conductivity;
  float ph;
  float air_temperature;
  float humidity;
  float distance;
  uint8_t hop;
  uint8_t source_id;
} SensorData;

SensorData data;
SensorData n1, n2;

float getDistM() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long t = pulseIn(ECHO, HIGH, 30000);
  if (t == 0) return 0;

  return (t * 0.0343 / 2.0) / 100.0;
}

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
void OnRecv(const esp_now_recv_info_t *info, const uint8_t *d, int len) {
#else
void OnRecv(const uint8_t *mac, const uint8_t *d, int len) {
#endif
  SensorData in;
  memcpy(&in, d, sizeof(in));

  if (in.node_id == 1) n1 = in;
  if (in.node_id == 2) n2 = in;
}

void sendData() {
  if (esp_now_send(HELTEC1, (uint8_t*)&data, sizeof(data)) != ESP_OK) {
    esp_now_send(NODE1, (uint8_t*)&data, sizeof(data));
    esp_now_send(NODE2, (uint8_t*)&data, sizeof(data));
  }
  Serial.println("NODE 3 SENT");
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(OnRecv);

  esp_now_peer_info_t p{};
  memcpy(p.peer_addr, HELTEC1, 6); esp_now_add_peer(&p);
  memcpy(p.peer_addr, NODE1,   6); esp_now_add_peer(&p);
  memcpy(p.peer_addr, NODE2,   6); esp_now_add_peer(&p);

  data.node_id = 3;
  data.source_id = 3;
}

void loop() {
  data.distance         = getDistM();

  data.soil_moisture    = 0;
  data.soil_temperature = 0;
  data.conductivity     = 0;
  data.ph               = 0;
  data.air_temperature  = 0;
  data.humidity         = 0;
  data.hop              = 0;

  sendData();

  Serial.println("\n===== NODE 3 VIEW =====");
  Serial.print("Distance: "); Serial.println(data.distance);
  Serial.println("======================");

  delay(4000);
}