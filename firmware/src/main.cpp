#include <Arduino.h>

#include "app/AppController.h"
#include "config/LogConfig.h"
#include "core/SerialLogger.h"
#include "devices/OledMonitor.h"

constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr const char *PROJECT_NAME = "NeuroHeart32";
constexpr const char *FIRMWARE_VERSION = "0.0.2";
constexpr unsigned long SERIAL_STARTUP_WAIT_MS = 10000;
constexpr unsigned long STARTUP_REPORT_DELAY_MS = 2000;

const LogConfig LOG = {
  true,  // main
  false, // oled
  false, // wifi
  false, // wifiScan
  false, // mqtt
  false, // menu
  false  // eeg
};

const OledMonitorConfig OLED_CONFIG = {
  5,             // sdaPin
  6,             // sclPin
  0x3C,          // i2cAddress
  SCREEN_WIDTH,  // width
  SCREEN_HEIGHT, // height
  LOG.oled       // loggingEnabled
};

SerialLogger logger;
OledMonitor oled(OLED_CONFIG, logger);

class SerialAppLogger : public AppLogger {
public:
  explicit SerialAppLogger(SerialLogger &logger) : _logger(logger) {
  }

  void mainInitializationStarted() override {
    _logger.println(F("MAIN | Inizializzazione su main"));
  }

  void mainFirmwareInfo(const char *projectName, const char *firmwareVersion) override {
    _logger.print(F("MAIN | Progetto: "));
    _logger.println(projectName);
    _logger.print(F("MAIN | Firmware: "));
    _logger.println(firmwareVersion);
  }

  void mainOledInitialized(uint8_t sdaPin, uint8_t sclPin, uint8_t i2cAddress) override {
    _logger.print(F("MAIN | OLED inizializzato | SDA: GPIO"));
    _logger.print(sdaPin);
    _logger.print(F(" | SCL: GPIO"));
    _logger.print(sclPin);
    _logger.print(F(" | address: 0x"));
    _logger.println(i2cAddress, HEX);
  }

  void mainOledInitializationFailed() override {
    _logger.println(F("MAIN | ERRORE: Display non trovato. Controlla le saldature!"));
  }

  void mainDelayedStartupReport(const char *projectName,
                                const char *firmwareVersion,
                                uint8_t sdaPin,
                                uint8_t sclPin,
                                bool oledReady) override {
    _logger.println(F("MAIN | Inizializzazione su main"));
    _logger.print(F("MAIN | Progetto: "));
    _logger.println(projectName);
    _logger.print(F("MAIN | Firmware: "));
    _logger.println(firmwareVersion);
    _logger.print(F("MAIN | OLED pin | SDA: GPIO"));
    _logger.print(sdaPin);
    _logger.print(F(" | SCL: GPIO"));
    _logger.println(sclPin);
    _logger.print(F("MAIN | OLED init: "));
    _logger.println(oledReady ? F("ok") : F("errore"));
  }

private:
  SerialLogger &_logger;
};

class OledMonitorAppAdapter : public AppOledMonitor {
public:
  OledMonitorAppAdapter(OledMonitor &oled, const OledMonitorConfig &config)
      : _oled(oled), _config(config) {
  }

  bool begin() override {
    return _oled.begin();
  }

  void clear() override {
    _oled.clear();
  }

  void drawFirmwareInfo(const char *projectName, const char *firmwareVersion) override {
    _oled.drawFirmwareInfo(projectName, firmwareVersion);
  }

  void printStatus() override {
    _oled.printStatus();
  }

  uint8_t sdaPin() const override {
    return _config.sdaPin;
  }

  uint8_t sclPin() const override {
    return _config.sclPin;
  }

  uint8_t i2cAddress() const override {
    return _config.i2cAddress;
  }

private:
  OledMonitor &_oled;
  const OledMonitorConfig &_config;
};

const AppControllerConfig APP_CONFIG = {
  PROJECT_NAME,
  FIRMWARE_VERSION,
  STARTUP_REPORT_DELAY_MS,
  LOG
};

SerialAppLogger appLogger(logger);
OledMonitorAppAdapter appOled(oled, OLED_CONFIG);
AppController app(APP_CONFIG, appOled, appLogger);

bool serialLoggingEnabled() {
  return isSerialLoggingEnabled(LOG);
}

void initializeSerialIfNeeded() {
  if (!serialLoggingEnabled()) {
    return;
  }

  Serial.begin(115200);
  const unsigned long startedAt = millis();
  while (!Serial && millis() - startedAt < SERIAL_STARTUP_WAIT_MS) {
    delay(10);
  }
}

void setup() {
  initializeSerialIfNeeded();
  app.setup();
}

void loop() {
  app.loop(millis());
}
