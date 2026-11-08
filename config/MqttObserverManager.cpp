#include "MqttObserverManager.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <helpers/ArduinoHelpers.h>

MqttObserverManager::MqttObserverManager()
    : _mesh(nullptr),
      _q_head(0),
      _q_tail(0),
      _q_count(0),
      _last_wifi_check(0),
      _last_status_publish(0),
      _packets_rx_count(0),
      _packets_tx_count(0),
      _last_snr(0.0f),
      _last_rssi(0.0f) {
  memset(_pub_key, 0, sizeof(_pub_key));
  memset(_pub_key_hex, 0, sizeof(_pub_key_hex));

  // Default global config
  _global_cfg.wifi_enabled = 0;
  _global_cfg.wifi_ssid[0] = 0;
  _global_cfg.wifi_pwd[0] = 0;
  _global_cfg.mqtt_enabled = 0;
  strcpy(_global_cfg.iata, "WAW");
  _global_cfg.status_enabled = 1;
  _global_cfg.packets_enabled = 1;
  _global_cfg.neighbors_enabled = 0;
  _global_cfg.status_interval_sec = 60;

  // Default slots
  for (int i = 0; i < MQTT_MAX_BROKERS; i++) {
    _slots[i].enabled = 0;
    _slots[i].server[0] = 0;
    _slots[i].port = 1883;
    _slots[i].user[0] = 0;
    _slots[i].pass[0] = 0;
    strcpy(_slots[i].topic_prefix, "meshcore");
    strcpy(_slots[i].preset, "none");
    _slots[i].packets_published = 0;
    _slots[i].last_connect_attempt = 0;
  }

  // Pre-seed known presets
  strcpy(_slots[0].preset, "meshmapper");
  strcpy(_slots[0].server, "mqtt.meshmapper.net");
  _slots[0].port = 1883;

  strcpy(_slots[1].preset, "corescope");
  strcpy(_slots[1].server, "mqtt.corescope.io");
  _slots[1].port = 1883;

  strcpy(_slots[2].preset, "none");
  strcpy(_slots[3].preset, "custom");
}

void MqttObserverManager::begin(mesh::Mesh* mesh, const uint8_t pub_key[PUB_KEY_SIZE]) {
  _mesh = mesh;
  memcpy(_pub_key, pub_key, PUB_KEY_SIZE);

  for (int i = 0; i < PUB_KEY_SIZE; i++) {
    sprintf(&_pub_key_hex[i * 2], "%02x", pub_key[i]);
  }
  _pub_key_hex[PUB_KEY_SIZE * 2] = 0;

  loadPrefs();

  for (int i = 0; i < MQTT_MAX_BROKERS; i++) {
    _mqtt_clients[i].setClient(_wifi_clients[i]);
    _mqtt_clients[i].setBufferSize(512);
    _mqtt_clients[i].setKeepAlive(20);
  }

  if (_global_cfg.wifi_enabled && _global_cfg.wifi_ssid[0]) {
    connectWifi();
  }
}

void MqttObserverManager::applyPreset(int idx, const char* preset_name) {
  if (idx < 0 || idx >= MQTT_MAX_BROKERS) return;

  if (strcasecmp(preset_name, "meshmapper") == 0) {
    strcpy(_slots[idx].preset, "meshmapper");
    strcpy(_slots[idx].server, "mqtt.meshmapper.net");
    _slots[idx].port = 1883;
    strcpy(_slots[idx].topic_prefix, "meshcore");
    _slots[idx].user[0] = 0;
    _slots[idx].pass[0] = 0;
    _slots[idx].enabled = 1;
  } else if (strcasecmp(preset_name, "corescope") == 0) {
    strcpy(_slots[idx].preset, "corescope");
    strcpy(_slots[idx].server, "mqtt.corescope.io");
    _slots[idx].port = 1883;
    strcpy(_slots[idx].topic_prefix, "meshcore");
    _slots[idx].user[0] = 0;
    _slots[idx].pass[0] = 0;
    _slots[idx].enabled = 1;
  } else if (strcasecmp(preset_name, "none") == 0) {
    strcpy(_slots[idx].preset, "none");
    _slots[idx].enabled = 0;
  } else if (strcasecmp(preset_name, "custom") == 0) {
    strcpy(_slots[idx].preset, "custom");
  }
}

void MqttObserverManager::connectWifi() {
  if (!_global_cfg.wifi_enabled || _global_cfg.wifi_ssid[0] == 0) {
    return;
  }
  Serial.printf("[MQTT-OBS] Connecting to Wi-Fi SSID: %s\n", _global_cfg.wifi_ssid);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(_global_cfg.wifi_ssid, _global_cfg.wifi_pwd);
  _last_wifi_check = millis();
}

void MqttObserverManager::maintainWifi() {
  if (!_global_cfg.wifi_enabled || _global_cfg.wifi_ssid[0] == 0) {
    return;
  }

  unsigned long now = millis();
  if (now - _last_wifi_check >= 5000) {
    _last_wifi_check = now;
    if (WiFi.status() != WL_CONNECTED) {
      if (WiFi.status() != WL_DISCONNECTED && WiFi.status() != WL_IDLE_STATUS) {
        WiFi.reconnect();
      }
    }
  }
}

void MqttObserverManager::maintainMqtt() {
  if (!_global_cfg.mqtt_enabled || WiFi.status() != WL_CONNECTED) {
    return;
  }

  unsigned long now = millis();

  for (int i = 0; i < MQTT_MAX_BROKERS; i++) {
    if (!_slots[i].enabled || _slots[i].server[0] == 0) {
      if (_mqtt_clients[i].connected()) {
        _mqtt_clients[i].disconnect();
      }
      continue;
    }

    if (_mqtt_clients[i].connected()) {
      _mqtt_clients[i].loop();
    } else {
      if (now - _slots[i].last_connect_attempt >= 10000) {
        _slots[i].last_connect_attempt = now;
        _mqtt_clients[i].setServer(_slots[i].server, _slots[i].port);

        char cid[48];
        snprintf(cid, sizeof(cid), "mc_c6l_%s_%d", &_pub_key_hex[PUB_KEY_SIZE * 2 - 8], i + 1);

        char lwt_topic[128];
        snprintf(lwt_topic, sizeof(lwt_topic), "%s/%s/%s/status",
                 _slots[i].topic_prefix[0] ? _slots[i].topic_prefix : "meshcore",
                 _global_cfg.iata[0] ? _global_cfg.iata : "WAW",
                 _pub_key_hex);

        const char* lwt_msg = "{\"status\":\"offline\"}";

        bool connected = false;
        if (_slots[i].user[0]) {
          connected = _mqtt_clients[i].connect(cid, _slots[i].user, _slots[i].pass, lwt_topic, 0, false, lwt_msg);
        } else {
          connected = _mqtt_clients[i].connect(cid, NULL, NULL, lwt_topic, 0, false, lwt_msg);
        }

        if (connected) {
          Serial.printf("[MQTT-OBS] Slot %d connected to %s:%d\n", i + 1, _slots[i].server, _slots[i].port);
          char status_topic[128];
          snprintf(status_topic, sizeof(status_topic), "%s/%s/%s/status",
                   _slots[i].topic_prefix[0] ? _slots[i].topic_prefix : "meshcore",
                   _global_cfg.iata[0] ? _global_cfg.iata : "WAW",
                   _pub_key_hex);

          char payload[320];
          snprintf(payload, sizeof(payload),
                   "{\"status\":\"online\",\"model\":\"M5Stack Unit C6L\",\"firmware\":\"1.18.0-repeater-observer\",\"uptime_secs\":%lu,\"free_heap\":%u}",
                   (unsigned long)(millis() / 1000), (unsigned int)esp_get_free_heap_size());

          _mqtt_clients[i].publish(status_topic, payload);
        }
      }
    }
  }
}

void MqttObserverManager::publishToBrokers(const char* subtopic, const char* json_payload) {
  if (!_global_cfg.mqtt_enabled) return;

  for (int i = 0; i < MQTT_MAX_BROKERS; i++) {
    if (_slots[i].enabled && _mqtt_clients[i].connected()) {
      char topic[128];
      snprintf(topic, sizeof(topic), "%s/%s/%s/%s",
               _slots[i].topic_prefix[0] ? _slots[i].topic_prefix : "meshcore",
               _global_cfg.iata[0] ? _global_cfg.iata : "WAW",
               _pub_key_hex,
               subtopic);

      if (_mqtt_clients[i].publish(topic, json_payload)) {
        _slots[i].packets_published++;
      }
    }
  }
}

void MqttObserverManager::flushQueue() {
  while (_q_count > 0) {
    MqttQueuedPacket& qp = _queue[_q_head];

    char hex_buf[(sizeof(qp.raw) * 2) + 1];
    int hex_len = qp.raw_len;
    if (hex_len > (int)sizeof(qp.raw)) hex_len = sizeof(qp.raw);
    for (int k = 0; k < hex_len; k++) {
      sprintf(&hex_buf[k * 2], "%02x", qp.raw[k]);
    }
    hex_buf[hex_len * 2] = 0;

    char payload[512];
    snprintf(payload, sizeof(payload),
             "{\"origin\":\"Unit-C6L\",\"origin_id\":\"%s\",\"timestamp\":%lu,\"type\":\"PACKET\","
             "\"direction\":\"rx\",\"len\":%u,\"packet_type\":%u,\"route\":\"%s\",\"payload_len\":%u,"
             "\"raw\":\"%s\",\"SNR\":%.2f,\"RSSI\":%.1f,\"score\":%d,\"hash\":\"%s\"}",
             _pub_key_hex,
             (unsigned long)qp.timestamp,
             qp.len,
             qp.payload_type,
             qp.is_direct ? "D" : "F",
             qp.payload_len,
             hex_buf,
             qp.snr,
             qp.rssi,
             (int)(qp.score * 1000),
             qp.hash_hex);

    publishToBrokers("packets", payload);

    _q_head = (_q_head + 1) % MQTT_QUEUE_SIZE;
    _q_count--;
  }
}

void MqttObserverManager::publishStatus() {
  if (!_global_cfg.mqtt_enabled || !_global_cfg.status_enabled) return;

  char payload[384];
  snprintf(payload, sizeof(payload),
           "{\"status\":\"online\",\"origin\":\"Unit-C6L\",\"origin_id\":\"%s\","
           "\"model\":\"M5Stack Unit C6L\",\"firmware\":\"1.18.0-repeater-observer\","
           "\"uptime_secs\":%lu,\"free_heap\":%u,\"wifi_rssi\":%d,\"wifi_ip\":\"%s\","
           "\"stats\":{\"packets_rx\":%lu,\"packets_tx\":%lu,\"last_snr\":%.2f,\"last_rssi\":%.1f}}",
           _pub_key_hex,
           (unsigned long)(millis() / 1000),
           (unsigned int)esp_get_free_heap_size(),
           WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0,
           WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString().c_str() : "0.0.0.0",
           (unsigned long)_packets_rx_count,
           (unsigned long)_packets_tx_count,
           _last_snr,
           _last_rssi);

  publishToBrokers("status", payload);
}

void MqttObserverManager::onPacketReceived(mesh::Packet* pkt, int len, float score, float snr, float rssi) {
  _packets_rx_count++;
  _last_snr = snr;
  _last_rssi = rssi;

  if (!_global_cfg.mqtt_enabled || !_global_cfg.packets_enabled) return;

  if (_q_count < MQTT_QUEUE_SIZE) {
    MqttQueuedPacket& qp = _queue[_q_tail];
    qp.timestamp = (uint32_t)(millis() / 1000);
    qp.snr = snr;
    qp.rssi = rssi;
    qp.score = score;
    qp.len = (uint8_t)(len > 255 ? 255 : len);
    qp.payload_len = (uint8_t)pkt->payload_len;
    qp.payload_type = (uint8_t)pkt->getPayloadType();
    qp.is_direct = pkt->isRouteDirect();

    uint8_t pkt_hash[MAX_HASH_SIZE];
    pkt->calculatePacketHash(pkt_hash);
    for (int k = 0; k < 4; k++) {
      sprintf(&qp.hash_hex[k * 2], "%02X", pkt_hash[k]);
    }
    qp.hash_hex[8] = 0;

    int copy_len = pkt->payload_len;
    if (copy_len > (int)sizeof(qp.raw)) copy_len = sizeof(qp.raw);
    memcpy(qp.raw, pkt->payload, copy_len);
    qp.raw_len = (uint8_t)copy_len;

    _q_tail = (_q_tail + 1) % MQTT_QUEUE_SIZE;
    _q_count++;
  }
}

void MqttObserverManager::onPacketTransmitted(mesh::Packet* pkt, int len) {
  _packets_tx_count++;
}

void MqttObserverManager::loop() {
  maintainWifi();

  if (WiFi.status() == WL_CONNECTED) {
    maintainMqtt();
    flushQueue();

    unsigned long now = millis();
    unsigned long interval_ms = (unsigned long)_global_cfg.status_interval_sec * 1000;
    if (interval_ms < 10000) interval_ms = 10000;

    if (now - _last_status_publish >= interval_ms) {
      _last_status_publish = now;
      publishStatus();
    }
  }
}

void MqttObserverManager::loadPrefs() {
  Preferences p;
  if (!p.begin("mc_mqtt", true)) {
    return;
  }

  _global_cfg.wifi_enabled = p.getUChar("w_en", _global_cfg.wifi_enabled);
  p.getString("w_ssid", _global_cfg.wifi_ssid, sizeof(_global_cfg.wifi_ssid));
  p.getString("w_pwd", _global_cfg.wifi_pwd, sizeof(_global_cfg.wifi_pwd));
  _global_cfg.mqtt_enabled = p.getUChar("m_en", _global_cfg.mqtt_enabled);
  p.getString("m_iata", _global_cfg.iata, sizeof(_global_cfg.iata));
  _global_cfg.status_enabled = p.getUChar("m_stat", _global_cfg.status_enabled);
  _global_cfg.packets_enabled = p.getUChar("m_pkt", _global_cfg.packets_enabled);
  _global_cfg.neighbors_enabled = p.getUChar("m_nbr", _global_cfg.neighbors_enabled);
  _global_cfg.status_interval_sec = p.getUShort("m_int", _global_cfg.status_interval_sec);

  for (int i = 0; i < MQTT_MAX_BROKERS; i++) {
    char key[16];
    snprintf(key, sizeof(key), "s%d_en", i + 1);
    _slots[i].enabled = p.getUChar(key, _slots[i].enabled);

    snprintf(key, sizeof(key), "s%d_srv", i + 1);
    p.getString(key, _slots[i].server, sizeof(_slots[i].server));

    snprintf(key, sizeof(key), "s%d_port", i + 1);
    _slots[i].port = p.getUShort(key, _slots[i].port);

    snprintf(key, sizeof(key), "s%d_usr", i + 1);
    p.getString(key, _slots[i].user, sizeof(_slots[i].user));

    snprintf(key, sizeof(key), "s%d_pwd", i + 1);
    p.getString(key, _slots[i].pass, sizeof(_slots[i].pass));

    snprintf(key, sizeof(key), "s%d_top", i + 1);
    p.getString(key, _slots[i].topic_prefix, sizeof(_slots[i].topic_prefix));

    snprintf(key, sizeof(key), "s%d_pre", i + 1);
    p.getString(key, _slots[i].preset, sizeof(_slots[i].preset));
  }
  p.end();
}

void MqttObserverManager::savePrefs() {
  Preferences p;
  if (!p.begin("mc_mqtt", false)) {
    return;
  }

  p.putUChar("w_en", _global_cfg.wifi_enabled);
  p.putString("w_ssid", _global_cfg.wifi_ssid);
  p.putString("w_pwd", _global_cfg.wifi_pwd);
  p.putUChar("m_en", _global_cfg.mqtt_enabled);
  p.putString("m_iata", _global_cfg.iata);
  p.putUChar("m_stat", _global_cfg.status_enabled);
  p.putUChar("m_pkt", _global_cfg.packets_enabled);
  p.putUChar("m_nbr", _global_cfg.neighbors_enabled);
  p.putUShort("m_int", _global_cfg.status_interval_sec);

  for (int i = 0; i < MQTT_MAX_BROKERS; i++) {
    char key[16];
    snprintf(key, sizeof(key), "s%d_en", i + 1);
    p.putUChar(key, _slots[i].enabled);

    snprintf(key, sizeof(key), "s%d_srv", i + 1);
    p.putString(key, _slots[i].server);

    snprintf(key, sizeof(key), "s%d_port", i + 1);
    p.putUShort(key, _slots[i].port);

    snprintf(key, sizeof(key), "s%d_usr", i + 1);
    p.putString(key, _slots[i].user);

    snprintf(key, sizeof(key), "s%d_pwd", i + 1);
    p.putString(key, _slots[i].pass);

    snprintf(key, sizeof(key), "s%d_top", i + 1);
    p.putString(key, _slots[i].topic_prefix);

    snprintf(key, sizeof(key), "s%d_pre", i + 1);
    p.putString(key, _slots[i].preset);
  }
  p.end();
}

bool MqttObserverManager::handleCommand(const char* command, char* reply) {
  // Wi-Fi commands
  if (strncmp(command, "set wifi.ssid ", 14) == 0) {
    strncpy(_global_cfg.wifi_ssid, &command[14], sizeof(_global_cfg.wifi_ssid) - 1);
    _global_cfg.wifi_ssid[sizeof(_global_cfg.wifi_ssid) - 1] = 0;
    savePrefs();
    if (_global_cfg.wifi_enabled && _global_cfg.wifi_ssid[0]) {
      connectWifi();
    }
    snprintf(reply, 160, "> wifi.ssid is now %s", _global_cfg.wifi_ssid);
    return true;
  }
  if (strncmp(command, "set wifi.pwd ", 13) == 0) {
    strncpy(_global_cfg.wifi_pwd, &command[13], sizeof(_global_cfg.wifi_pwd) - 1);
    _global_cfg.wifi_pwd[sizeof(_global_cfg.wifi_pwd) - 1] = 0;
    savePrefs();
    if (_global_cfg.wifi_enabled && _global_cfg.wifi_ssid[0]) {
      connectWifi();
    }
    strcpy(reply, "> wifi.pwd updated and reconnecting");
    return true;
  }
  if (strncmp(command, "set wifi.enabled ", 17) == 0) {
    _global_cfg.wifi_enabled = atoi(&command[17]) ? 1 : 0;
    savePrefs();
    if (_global_cfg.wifi_enabled) {
      connectWifi();
    } else {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
    }
    snprintf(reply, 160, "> wifi.enabled is now %d", _global_cfg.wifi_enabled);
    return true;
  }
  if (strcmp(command, "get wifi.status") == 0) {
    const char* st_str = "DISCONNECTED";
    if (WiFi.status() == WL_CONNECTED) st_str = "CONNECTED";
    else if (WiFi.status() == WL_CONNECT_FAILED) st_str = "FAILED";
    else if (WiFi.status() == WL_NO_SSID_AVAIL) st_str = "SSID_NOT_FOUND";

    snprintf(reply, 160, "> Wi-Fi: %s (SSID: %s, IP: %s, RSSI: %d dBm)",
             st_str,
             _global_cfg.wifi_ssid[0] ? _global_cfg.wifi_ssid : "NONE",
             WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString().c_str() : "0.0.0.0",
             WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0);
    return true;
  }
  if (strcmp(command, "get wifi.ssid") == 0) {
    snprintf(reply, 160, "> wifi.ssid = %s", _global_cfg.wifi_ssid[0] ? _global_cfg.wifi_ssid : "(empty)");
    return true;
  }
  if (strcmp(command, "get wifi.enabled") == 0) {
    snprintf(reply, 160, "> wifi.enabled = %d", _global_cfg.wifi_enabled);
    return true;
  }

  // Global MQTT commands
  if (strncmp(command, "set mqtt.enabled ", 17) == 0) {
    _global_cfg.mqtt_enabled = atoi(&command[17]) ? 1 : 0;
    savePrefs();
    snprintf(reply, 160, "> mqtt.enabled is now %d", _global_cfg.mqtt_enabled);
    return true;
  }
  if (strncmp(command, "set mqtt.iata ", 14) == 0) {
    const char* code = &command[14];
    while (*code == ' ') code++;
    int len = 0;
    while (code[len] && code[len] != ' ' && code[len] != '\r' && code[len] != '\n' && len < 7) {
      _global_cfg.iata[len] = toupper(code[len]);
      len++;
    }
    _global_cfg.iata[len] = 0;
    savePrefs();
    snprintf(reply, 160, "> mqtt.iata is now %s", _global_cfg.iata);
    return true;
  }
  if (strncmp(command, "set mqtt.status ", 16) == 0) {
    _global_cfg.status_enabled = atoi(&command[16]) ? 1 : 0;
    savePrefs();
    snprintf(reply, 160, "> mqtt.status is now %d", _global_cfg.status_enabled);
    return true;
  }
  if (strncmp(command, "set mqtt.packets ", 17) == 0) {
    _global_cfg.packets_enabled = atoi(&command[17]) ? 1 : 0;
    savePrefs();
    snprintf(reply, 160, "> mqtt.packets is now %d", _global_cfg.packets_enabled);
    return true;
  }
  if (strncmp(command, "set mqtt.neighbors ", 20) == 0) {
    _global_cfg.neighbors_enabled = atoi(&command[20]) ? 1 : 0;
    savePrefs();
    snprintf(reply, 160, "> mqtt.neighbors is now %d", _global_cfg.neighbors_enabled);
    return true;
  }
  if (strncmp(command, "set mqtt.interval ", 18) == 0) {
    int val = atoi(&command[18]);
    if (val < 10) val = 10;
    _global_cfg.status_interval_sec = (uint16_t)val;
    savePrefs();
    snprintf(reply, 160, "> mqtt.interval is now %d sec", _global_cfg.status_interval_sec);
    return true;
  }
  if (strcmp(command, "get mqtt.status") == 0) {
    int conn_count = 0;
    for (int i = 0; i < MQTT_MAX_BROKERS; i++) {
      if (_slots[i].enabled && _mqtt_clients[i].connected()) conn_count++;
    }
    snprintf(reply, 160, "> MQTT: %s | Active Brokers: %d/%d | IATA: %s | Rx: %lu, Tx: %lu",
             _global_cfg.mqtt_enabled ? "ENABLED" : "DISABLED",
             conn_count, MQTT_MAX_BROKERS,
             _global_cfg.iata[0] ? _global_cfg.iata : "UNSET",
             (unsigned long)_packets_rx_count, (unsigned long)_packets_tx_count);
    return true;
  }
  if (strcmp(command, "get mqtt.iata") == 0) {
    snprintf(reply, 160, "> mqtt.iata = %s", _global_cfg.iata[0] ? _global_cfg.iata : "UNSET");
    return true;
  }
  if (strcmp(command, "get mqtt.config") == 0) {
    snprintf(reply, 160, "> Global: en=%d, iata=%s, stat=%d, pkts=%d, int=%ds",
             _global_cfg.mqtt_enabled, _global_cfg.iata, _global_cfg.status_enabled,
             _global_cfg.packets_enabled, _global_cfg.status_interval_sec);
    return true;
  }

  // Per-slot commands: mqtt1..mqtt4
  for (int s = 1; s <= MQTT_MAX_BROKERS; s++) {
    char prefix[16];
    snprintf(prefix, sizeof(prefix), "set mqtt%d.", s);
    int plen = strlen(prefix);

    if (strncmp(command, prefix, plen) == 0) {
      const char* sub = command + plen;
      int idx = s - 1;

      if (strncmp(sub, "enabled ", 8) == 0) {
        _slots[idx].enabled = atoi(sub + 8) ? 1 : 0;
        savePrefs();
        snprintf(reply, 160, "> mqtt%d.enabled = %d", s, _slots[idx].enabled);
        return true;
      }
      if (strncmp(sub, "server ", 7) == 0) {
        strncpy(_slots[idx].server, sub + 7, sizeof(_slots[idx].server) - 1);
        _slots[idx].server[sizeof(_slots[idx].server) - 1] = 0;
        savePrefs();
        snprintf(reply, 160, "> mqtt%d.server = %s", s, _slots[idx].server);
        return true;
      }
      if (strncmp(sub, "port ", 5) == 0) {
        _slots[idx].port = (uint16_t)atoi(sub + 5);
        savePrefs();
        snprintf(reply, 160, "> mqtt%d.port = %d", s, _slots[idx].port);
        return true;
      }
      if (strncmp(sub, "user ", 5) == 0) {
        strncpy(_slots[idx].user, sub + 5, sizeof(_slots[idx].user) - 1);
        _slots[idx].user[sizeof(_slots[idx].user) - 1] = 0;
        savePrefs();
        snprintf(reply, 160, "> mqtt%d.user = %s", s, _slots[idx].user);
        return true;
      }
      if (strncmp(sub, "pass ", 5) == 0) {
        strncpy(_slots[idx].pass, sub + 5, sizeof(_slots[idx].pass) - 1);
        _slots[idx].pass[sizeof(_slots[idx].pass) - 1] = 0;
        savePrefs();
        snprintf(reply, 160, "> mqtt%d.pass updated", s);
        return true;
      }
      if (strncmp(sub, "topic ", 6) == 0) {
        strncpy(_slots[idx].topic_prefix, sub + 6, sizeof(_slots[idx].topic_prefix) - 1);
        _slots[idx].topic_prefix[sizeof(_slots[idx].topic_prefix) - 1] = 0;
        savePrefs();
        snprintf(reply, 160, "> mqtt%d.topic = %s", s, _slots[idx].topic_prefix);
        return true;
      }
      if (strncmp(sub, "preset ", 7) == 0) {
        applyPreset(idx, sub + 7);
        savePrefs();
        snprintf(reply, 160, "> mqtt%d.preset = %s (srv: %s:%d, en: %d)",
                 s, _slots[idx].preset, _slots[idx].server, _slots[idx].port, _slots[idx].enabled);
        return true;
      }
    }

    // get mqtt<N>
    char get_cmd[16];
    snprintf(get_cmd, sizeof(get_cmd), "get mqtt%d", s);
    if (strcmp(command, get_cmd) == 0) {
      int idx = s - 1;
      const char* conn_str = _mqtt_clients[idx].connected() ? "CONNECTED" : "OFFLINE";
      snprintf(reply, 160, "> mqtt%d: [%s] %s:%d (topic: %s, preset: %s, sent: %lu)",
               s, conn_str,
               _slots[idx].server[0] ? _slots[idx].server : "(none)",
               _slots[idx].port,
               _slots[idx].topic_prefix,
               _slots[idx].preset,
               (unsigned long)_slots[idx].packets_published);
      return true;
    }
  }

  return false;
}
