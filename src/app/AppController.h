#pragma once

#include <stdint.h>

#include "../config/LogConfig.h"

class AppLogger {
public:
  virtual ~AppLogger() = default;

  virtual void mainInitializationStarted() = 0;
  virtual void mainFirmwareInfo(const char *firmwareVersion) = 0;
  virtual void mainOledInitialized(uint8_t sdaPin, uint8_t sclPin, uint8_t i2cAddress) = 0;
  virtual void mainOledInitializationFailed() = 0;
  virtual void mainDelayedStartupReport(const char *firmwareVersion,
                                        uint8_t sdaPin,
                                        uint8_t sclPin,
                                        bool oledReady) = 0;
};

class AppOledMonitor {
public:
  virtual ~AppOledMonitor() = default;

  virtual bool begin() = 0;
  virtual void clear() = 0;
  virtual void drawFirmwareInfo(const char *firmwareVersion) = 0;
  virtual void printStatus() = 0;
  virtual uint8_t sdaPin() const = 0;
  virtual uint8_t sclPin() const = 0;
  virtual uint8_t i2cAddress() const = 0;
};

struct AppControllerConfig {
  const char *firmwareVersion;
  unsigned long startupReportDelayMs;
  LogConfig log;
};

class AppController {
public:
  AppController(const AppControllerConfig &config, AppOledMonitor &oled, AppLogger &logger);

  void setup();
  void loop(unsigned long nowMs);

  bool oledReady() const;
  bool startupReportPrinted() const;

private:
  bool mainLoggingEnabled() const;

  AppControllerConfig _config;
  AppOledMonitor &_oled;
  AppLogger &_logger;
  bool _oledReady;
  bool _startupReportPrinted;
};
