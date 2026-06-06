#include "WifiManager.h"

WifiManager *WifiManager::_activeInstance = nullptr;

WifiManager::WifiManager(const WifiManagerConfig &config, LoggerInterface &logger)
    : _config(config),
      _logger(logger),
      _uiState(WIFI_UI_DISCONNECTED),
      _lastDisconnectReason(-1) {
}

bool WifiManager::connect(ProgressCallback onProgress) {
  _uiState = WIFI_UI_CONNECTING;
  _lastDisconnectReason = -1;
  _activeInstance = this;

  if (_config.loggingEnabled) {
    _logger.println(F("WIFI | avvio connessione"));
    _logger.print(F("WIFI | SSID: "));
    _logger.println(_config.ssid);
    _logger.print(F("WIFI | timeout ms: "));
    _logger.println(_config.connectTimeoutMs);
  }

  WiFi.onEvent(WifiManager::onWifiEvent);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.disconnect(true, true);
  delay(300);

  if (_config.loggingEnabled) {
    _logger.print(F("WIFI | MAC STA: "));
    const String macAddress = WiFi.macAddress();
    _logger.println(macAddress.c_str());
  }

  scanForDebug();
  WiFi.begin(_config.ssid, _config.password);

  const unsigned long startedAt = millis();
  unsigned long lastLogAt = 0;
  wl_status_t lastStatus = WL_IDLE_STATUS;

  while (millis() - startedAt < _config.connectTimeoutMs) {
    const unsigned long now = millis();
    const wl_status_t status = WiFi.status();

    if (status != lastStatus || now - lastLogAt >= _config.connectLogIntervalMs) {
      if (_config.loggingEnabled) {
        _logger.print(F("WIFI | stato: "));
        _logger.print(statusName(status));
        _logger.print(F(" | elapsed ms: "));
        _logger.println(now - startedAt);
      }
      lastStatus = status;
      lastLogAt = now;
    }

    if (status == WL_CONNECTED) {
      _uiState = WIFI_UI_CONNECTED;
      if (_config.loggingEnabled) {
        _logger.println(F("WIFI | connesso"));
        _logger.print(F("WIFI | IP: "));
        const String ipAddress = WiFi.localIP().toString();
        _logger.println(ipAddress.c_str());
        _logger.print(F("WIFI | RSSI: "));
        _logger.println(WiFi.RSSI());
      }
      return true;
    }

    if (onProgress != nullptr) {
      onProgress(now - startedAt);
    }

    delay(50);
  }

  _uiState = WIFI_UI_DISCONNECTED;
  if (_config.loggingEnabled) {
    _logger.println(F("WIFI | timeout, connessione non riuscita"));
    _logger.print(F("WIFI | ultimo stato: "));
    _logger.println(statusName());
    _logger.print(F("WIFI | ultimo motivo disconnessione: "));
    _logger.print((int)_lastDisconnectReason);
    _logger.print(F(" ("));
    _logger.print(disconnectReasonName(_lastDisconnectReason));
    _logger.println(F(")"));
  }
  WiFi.disconnect(false);
  return false;
}

bool WifiManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

WifiUiState WifiManager::uiState() const {
  return _uiState;
}

const char *WifiManager::statusName() const {
  return statusName(WiFi.status());
}

const char *WifiManager::statusName(wl_status_t status) const {
  switch (status) {
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED:
      return "WL_SCAN_COMPLETED";
    case WL_CONNECTED:
      return "WL_CONNECTED";
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED";
    default:
      return "WL_STATUS_SCONOSCIUTO";
  }
}

void WifiManager::onWifiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  if (_activeInstance != nullptr) {
    _activeInstance->handleWifiEvent(event, info);
  }
}

void WifiManager::scanForDebug() {
  if (!_config.scanLoggingEnabled) {
    return;
  }

  _logger.println(F("WIFI | scan avvio"));
  const int networkCount = WiFi.scanNetworks(false, true);

  if (networkCount < 0) {
    _logger.print(F("WIFI | scan errore: "));
    _logger.println(networkCount);
    return;
  }

  _logger.print(F("WIFI | scan reti trovate: "));
  _logger.println(networkCount);

  bool foundConfiguredSsid = false;
  for (int i = 0; i < networkCount; i++) {
    const String ssid = WiFi.SSID(i);
    const bool isConfiguredSsid = ssid == _config.ssid;

    if (isConfiguredSsid) {
      foundConfiguredSsid = true;
    }

    _logger.print(F("WIFI | scan "));
    _logger.print(isConfiguredSsid ? F("* ") : F("  "));
    _logger.print(ssid.c_str());
    _logger.print(F(" | RSSI: "));
    _logger.print((int)WiFi.RSSI(i));
    _logger.print(F(" | channel: "));
    _logger.print((int)WiFi.channel(i));
    _logger.print(F(" | encryption: "));
    _logger.println(WiFi.encryptionType(i));
  }

  _logger.print(F("WIFI | scan SSID configurato trovato: "));
  _logger.println(foundConfiguredSsid ? F("si") : F("no"));
  WiFi.scanDelete();
}

const char *WifiManager::disconnectReasonName(int reason) const {
  switch (reason) {
    case 2:
      return "AUTH_EXPIRE";
    case 4:
      return "ASSOC_EXPIRE";
    case 8:
      return "ASSOC_LEAVE";
    case 15:
      return "4WAY_HANDSHAKE_TIMEOUT";
    case 202:
      return "AUTH_FAIL";
    case 201:
      return "NO_AP_FOUND";
    case 204:
      return "HANDSHAKE_TIMEOUT";
    default:
      return "SCONOSCIUTO";
  }
}

void WifiManager::handleWifiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  if (!_config.loggingEnabled) {
    return;
  }

  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      _logger.println(F("WIFI | event STA_START"));
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      _logger.println(F("WIFI | event STA_CONNECTED"));
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
      _logger.print(F("WIFI | event GOT_IP | IP: "));
      const String ipAddress = WiFi.localIP().toString();
      _logger.println(ipAddress.c_str());
      break;
    }
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      _lastDisconnectReason = info.wifi_sta_disconnected.reason;
      _logger.print(F("WIFI | event STA_DISCONNECTED | reason: "));
      _logger.print((int)_lastDisconnectReason);
      _logger.print(F(" ("));
      _logger.print(disconnectReasonName(_lastDisconnectReason));
      _logger.println(F(")"));
      break;
    default:
      break;
  }
}
