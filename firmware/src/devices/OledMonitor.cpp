#include "OledMonitor.h"

OledMonitor::OledMonitor(const OledMonitorConfig &config, LoggerInterface &logger)
    : _config(config),
      _logger(logger),
      _display(config.width, config.height, &Wire, -1),
      _ready(false),
      _address(0) {
}

bool OledMonitor::begin() {
  configureBus();

  _ready = false;
  _address = _config.i2cAddress;

  if (_config.loggingEnabled) {
    _logger.print(F("OLED | provo indirizzo configurato 0x"));
    _logger.println(_address, HEX);
  }

  if (!i2cDeviceResponds(_address)) {
    if (_config.loggingEnabled) {
      _logger.println(F("OLED | nessuna risposta I2C all'indirizzo configurato"));
    }
    _ready = false;
    return false;
  }

  if (_display.begin(SSD1306_SWITCHCAPVCC, _address)) {
    _ready = true;
    if (_config.loggingEnabled) {
      _logger.print(F("OLED | inizializzato a 0x"));
      _logger.println(_address, HEX);
    }
    return true;
  }

  if (_config.loggingEnabled) {
    _logger.println(F("OLED | inizializzazione fallita"));
  }
  return false;
}

void OledMonitor::configureBus() {
  logConfig("configured");

  pinMode(_config.sdaPin, INPUT_PULLUP);
  pinMode(_config.sclPin, INPUT_PULLUP);
  Wire.end();
  Wire.begin(_config.sdaPin, _config.sclPin);
  Wire.setClock(100000);
  delay(50);
}

uint8_t OledMonitor::scanBus(const char *label) {
  if (_config.loggingEnabled) {
    _logger.print(F("OLED | I2C scan avvio | "));
    _logger.println(label);
  }

  uint8_t deviceCount = 0;
  for (uint8_t address = 1; address < 127; address++) {
    if (i2cDeviceResponds(address)) {
      if (_config.loggingEnabled) {
        _logger.print(F("OLED | I2C dispositivo trovato a 0x"));
        if (address < 16) {
          _logger.print(F("0"));
        }
        _logger.println(address, HEX);
      }
      deviceCount++;
    }
  }

  if (_config.loggingEnabled && deviceCount == 0) {
    _logger.println(F("OLED | I2C nessun dispositivo trovato"));
  }

  return deviceCount;
}

void OledMonitor::clear() {
  if (!_ready) {
    return;
  }

  _display.clearDisplay();
  _display.display();
}

void OledMonitor::printStatus() {
  if (!_config.loggingEnabled) {
    return;
  }

  _logger.print(F("OLED | stato: "));
  if (!_ready || _address == 0) {
    _logger.println(F("non inizializzato"));
    return;
  }

  if (!i2cDeviceResponds(_address)) {
    _logger.print(F("perso/non visto su I2C | ultimo indirizzo: 0x"));
    _logger.println(_address, HEX);
    return;
  }

  _logger.print(F("ok | indirizzo: 0x"));
  _logger.println(_address, HEX);
}

void OledMonitor::drawTestScreen() {
  if (!_ready) {
    return;
  }

  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE);
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.println(F("OLED TEST"));
  _display.println(F("I2C OK"));
  _display.print(F("Addr: 0x"));
  _display.println(_address, HEX);
  _display.drawRect(0, 32, _config.width, 16, SSD1306_WHITE);
  _display.fillRect(4, 36, _config.width - 8, 8, SSD1306_WHITE);
  _display.display();
}

void OledMonitor::drawFirmwareInfo(const char *projectName, const char *firmwareVersion) {
  if (!_ready) {
    return;
  }

  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE);
  _display.setTextSize(1);

  _display.setCursor(0, 0);
  _display.println(projectName);

  _display.setCursor(0, 16);
  _display.print(F("Firmware: "));
  _display.println(firmwareVersion);

  _display.setCursor(0, 32);
  _display.print(F("OLED SDA: GPIO"));
  _display.println(_config.sdaPin);

  _display.setCursor(0, 44);
  _display.print(F("OLED SCL: GPIO"));
  _display.println(_config.sclPin);

  _display.display();
}

bool OledMonitor::isReady() const {
  return _ready;
}

uint8_t OledMonitor::address() const {
  return _ready ? _address : _config.i2cAddress;
}

Adafruit_SSD1306 &OledMonitor::display() {
  return _display;
}

bool OledMonitor::i2cDeviceResponds(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

void OledMonitor::logConfig(const char *label) {
  if (!_config.loggingEnabled) {
    return;
  }

  _logger.print(F("OLED | I2C config "));
  _logger.print(label);
  _logger.print(F(" | SDA: GPIO"));
  _logger.print(_config.sdaPin);
  _logger.print(F(" | SCL: GPIO"));
  _logger.println(_config.sclPin);
}
