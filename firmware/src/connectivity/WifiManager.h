#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include "../core/LoggerInterface.h"

enum WifiUiState {
  WIFI_UI_DISCONNECTED,
  WIFI_UI_CONNECTING,
  WIFI_UI_CONNECTED
};

struct WifiManagerConfig {
  const char *ssid;
  const char *password;
  unsigned long connectTimeoutMs;
  unsigned long connectLogIntervalMs;
  bool loggingEnabled;
  bool scanLoggingEnabled;
};

class WifiManager {
public:
  using ProgressCallback = void (*)(unsigned long elapsedMs);

  WifiManager(const WifiManagerConfig &config, LoggerInterface &logger);

  bool connect(ProgressCallback onProgress = nullptr);
  bool isConnected() const;
  WifiUiState uiState() const;
  const char *statusName() const;
  const char *statusName(wl_status_t status) const;

private:
  static void onWifiEvent(WiFiEvent_t event, WiFiEventInfo_t info);

  void scanForDebug();
  const char *disconnectReasonName(int reason) const;
  void handleWifiEvent(WiFiEvent_t event, WiFiEventInfo_t info);

  WifiManagerConfig _config;
  LoggerInterface &_logger;
  WifiUiState _uiState;
  volatile int _lastDisconnectReason;

  static WifiManager *_activeInstance;
};
