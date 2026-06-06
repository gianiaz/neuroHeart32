#include "MqttBroker.h"

MqttBroker::MqttBroker(const MqttBrokerConfig &config, LoggerInterface &logger)
    : _config(config),
      _logger(logger),
      _wifiClient(),
      _client(_wifiClient),
      _lastConnectAttemptAt(0) {
  _client.setServer(_config.host, _config.port);
}

bool MqttBroker::connect() {
  if (_client.connected()) {
    return true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    if (_config.loggingEnabled) {
      _logger.println(F("MQTT | WiFi non connesso, impossibile connettere il broker"));
    }
    return false;
  }

  const unsigned long now = millis();
  if (_lastConnectAttemptAt != 0 && now - _lastConnectAttemptAt < _config.reconnectIntervalMs) {
    if (_config.loggingEnabled) {
      _logger.print(F("MQTT | riconnessione in cooldown, ms rimanenti: "));
      _logger.println(_config.reconnectIntervalMs - (now - _lastConnectAttemptAt));
    }
    return false;
  }

  _lastConnectAttemptAt = now;

  if (_config.loggingEnabled) {
    _logger.println(F("MQTT | avvio connessione"));
    _logger.print(F("MQTT | host: "));
    _logger.print(_config.host);
    _logger.print(F(":"));
    _logger.println((int)_config.port);
    _logger.print(F("MQTT | user: "));
    _logger.println(_config.user);
    _logger.println(F("MQTT | tentativo singolo"));
  }

  if (_client.connect(_config.clientId, _config.user, _config.password)) {
    if (_config.loggingEnabled) {
      _logger.println(F("MQTT | connesso"));
    }
    return true;
  }

  const int state = _client.state();
  if (_config.loggingEnabled) {
    _logger.print(F("MQTT | connessione non riuscita, ultimo state: "));
    _logger.print(state);
    _logger.print(F(" ("));
    _logger.print(stateName(state));
    _logger.println(F(")"));
  }

  if (_config.loggingEnabled && (state == MQTT_CONNECT_BAD_CREDENTIALS || state == MQTT_CONNECT_UNAUTHORIZED)) {
    _logger.println(F("MQTT | errore autenticazione, controllo user/password o permessi broker necessario"));
  }

  return false;
}

void MqttBroker::loop() {
  _client.loop();
}

bool MqttBroker::publish(const char *topic, const char *payload, size_t payloadSize) {
  if (!connect()) {
    if (_config.loggingEnabled) {
      _logger.println(F("MQTT | errore, broker non connesso"));
    }
    return false;
  }

  const bool published = _client.publish(topic, payload, payloadSize);
  if (_config.loggingEnabled) {
    _logger.print(F("MQTT | publish topic: "));
    _logger.print(topic);
    _logger.print(F(" | bytes: "));
    _logger.print((int)payloadSize);
    _logger.print(F(" | esito: "));
    _logger.println(published ? F("ok") : F("fallito"));
  }

  return published;
}

bool MqttBroker::connected() {
  return _client.connected();
}

const char *MqttBroker::stateName() {
  return stateName(_client.state());
}

const char *MqttBroker::stateName(int state) const {
  switch (state) {
    case MQTT_CONNECTION_TIMEOUT:
      return "MQTT_CONNECTION_TIMEOUT";
    case MQTT_CONNECTION_LOST:
      return "MQTT_CONNECTION_LOST";
    case MQTT_CONNECT_FAILED:
      return "MQTT_CONNECT_FAILED";
    case MQTT_DISCONNECTED:
      return "MQTT_DISCONNECTED";
    case MQTT_CONNECTED:
      return "MQTT_CONNECTED";
    case MQTT_CONNECT_BAD_PROTOCOL:
      return "MQTT_CONNECT_BAD_PROTOCOL";
    case MQTT_CONNECT_BAD_CLIENT_ID:
      return "MQTT_CONNECT_BAD_CLIENT_ID";
    case MQTT_CONNECT_UNAVAILABLE:
      return "MQTT_CONNECT_UNAVAILABLE";
    case MQTT_CONNECT_BAD_CREDENTIALS:
      return "MQTT_CONNECT_BAD_CREDENTIALS";
    case MQTT_CONNECT_UNAUTHORIZED:
      return "MQTT_CONNECT_UNAUTHORIZED";
    default:
      return "MQTT_STATE_SCONOSCIUTO";
  }
}
