#pragma once

struct LogConfig {
  bool main;
  bool oled;
  bool wifi;
  bool wifiScan;
  bool mqtt;
  bool menu;
  bool eeg;
};

inline bool isSerialLoggingEnabled(const LogConfig &config) {
  return config.main ||
         config.oled ||
         config.wifi ||
         config.wifiScan ||
         config.mqtt ||
         config.menu ||
         config.eeg;
}
