#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <Mesh.h>

#define MQTT_MAX_BROKERS 4
#define MQTT_QUEUE_SIZE 16

struct MqttSlotConfig {
  uint8_t enabled;
  char server[64];
  uint16_t port;
  char user[32];
  char pass[32];
  char topic_prefix[32];
  char preset[16];
  uint32_t packets_published;
  unsigned long last_connect_attempt;
};

struct MqttGlobalConfig {
  uint8_t wifi_enabled;
  char wifi_ssid[33];
  char wifi_pwd[65];
  uint8_t mqtt_enabled;
  char iata[8];
  uint8_t status_enabled;
  uint8_t packets_enabled;
  uint8_t neighbors_enabled;
  uint16_t status_interval_sec;
};

struct MqttQueuedPacket {
  uint32_t timestamp;
  float snr;
  float rssi;
  float score;
  uint8_t len;
  uint8_t payload_len;
  uint8_t payload_type;
  bool is_direct;
  char hash_hex[17];
  uint8_t raw[MAX_PACKET_PAYLOAD + 16];
  uint8_t raw_len;
};

class MqttObserverManager {
private:
  mesh::Mesh* _mesh;
  uint8_t _pub_key[PUB_KEY_SIZE];
  char _pub_key_hex[PUB_KEY_SIZE * 2 + 1];

  MqttGlobalConfig _global_cfg;
  MqttSlotConfig _slots[MQTT_MAX_BROKERS];

  WiFiClient _wifi_clients[MQTT_MAX_BROKERS];
  PubSubClient _mqtt_clients[MQTT_MAX_BROKERS];

  MqttQueuedPacket _queue[MQTT_QUEUE_SIZE];
  uint8_t _q_head;
  uint8_t _q_tail;
  uint8_t _q_count;

  unsigned long _last_wifi_check;
  unsigned long _last_status_publish;
  uint32_t _packets_rx_count;
  uint32_t _packets_tx_count;
  float _last_snr;
  float _last_rssi;

  void applyPreset(int slot_idx, const char* preset_name);
  void connectWifi();
  void maintainWifi();
  void maintainMqtt();
  void flushQueue();
  void publishToBrokers(const char* subtopic, const char* json_payload);

public:
  MqttObserverManager();
  void begin(mesh::Mesh* mesh, const uint8_t pub_key[PUB_KEY_SIZE]);
  void loop();
  bool handleCommand(const char* command, char* reply);

  void onPacketReceived(mesh::Packet* pkt, int len, float score, float snr, float rssi);
  void onPacketTransmitted(mesh::Packet* pkt, int len);

  void loadPrefs();
  void savePrefs();
  void publishStatus();

  bool isWifiConnected() const { return WiFi.status() == WL_CONNECTED; }
  bool isSlotConnected(int idx) {
    if (idx < 0 || idx >= MQTT_MAX_BROKERS) return false;
    return _mqtt_clients[idx].connected();
  }
};
