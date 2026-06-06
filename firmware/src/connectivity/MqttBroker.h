#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "../core/LoggerInterface.h"

struct MqttBrokerConfig {
  const char *host;
  uint16_t port;
  const char *user;
  const char *password;
  const char *clientId;
  unsigned long reconnectIntervalMs;
  bool loggingEnabled;
};

class MqttBroker {
public:
  MqttBroker(const MqttBrokerConfig &config, LoggerInterface &logger);

  bool connect();
  void loop();
  bool publish(const char *topic, const char *payload, size_t payloadSize);
  bool connected();
  const char *stateName();

private:
  const char *stateName(int state) const;

  MqttBrokerConfig _config;
  LoggerInterface &_logger;
  WiFiClient _wifiClient;
  PubSubClient _client;
  unsigned long _lastConnectAttemptAt;
};
