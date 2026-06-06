#include "AppController.h"

AppController::AppController(const AppControllerConfig &config, AppOledMonitor &oled, AppLogger &logger)
    : _config(config),
      _oled(oled),
      _logger(logger),
      _oledReady(false),
      _startupReportPrinted(false) {
}

void AppController::setup() {
  if (mainLoggingEnabled()) {
    _logger.mainInitializationStarted();
  }

  _oledReady = _oled.begin();
  if (!_oledReady) {
    if (mainLoggingEnabled()) {
      _logger.mainOledInitializationFailed();
    }
    return;
  }

  _oled.clear();

  if (mainLoggingEnabled()) {
    _logger.mainFirmwareInfo(_config.firmwareVersion);
    _logger.mainOledInitialized(_oled.sdaPin(), _oled.sclPin(), _oled.i2cAddress());
  }

  _oled.drawFirmwareInfo(_config.firmwareVersion);
  _oled.printStatus();
}

void AppController::loop(unsigned long nowMs) {
  if (_startupReportPrinted || nowMs < _config.startupReportDelayMs) {
    return;
  }

  _startupReportPrinted = true;
  if (mainLoggingEnabled()) {
    _logger.mainDelayedStartupReport(_config.firmwareVersion, _oled.sdaPin(), _oled.sclPin(), _oledReady);
  }
}

bool AppController::oledReady() const {
  return _oledReady;
}

bool AppController::startupReportPrinted() const {
  return _startupReportPrinted;
}

bool AppController::mainLoggingEnabled() const {
  return _config.log.main;
}
