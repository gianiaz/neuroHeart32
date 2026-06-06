#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "../core/LoggerInterface.h"

struct OledMonitorConfig {
  uint8_t sdaPin;
  uint8_t sclPin;
  uint8_t i2cAddress;
  uint8_t width;
  uint8_t height;
  bool loggingEnabled;
};

// Gestisce un display OLED SSD1306 collegato su bus I2C.
// Nel firmware normale usa la configurazione esplicita passata dal main
// (pin e indirizzo); i metodi di scan sono disponibili solo per diagnostica.
class OledMonitor {
public:
  OledMonitor(const OledMonitorConfig &config, LoggerInterface &logger);

  // Configura il bus I2C con i pin dichiarati in OledMonitorConfig.
  void configureBus();

  // Inizializza il display all'indirizzo I2C configurato, senza fare scan.
  bool begin();

  // Metodo diagnostico: scandisce il bus I2C gia configurato e logga i device trovati.
  uint8_t scanBus(const char *label = "manual scan");

  // Spegne tutti i pixel del display inizializzato.
  void clear();

  // Metodo diagnostico: stampa lo stato corrente del display e della connessione I2C.
  void printStatus();

  // Schermata diagnostica generica per verificare rapidamente che il display scriva pixel.
  void drawTestScreen();

  // Schermata iniziale minima usata nel bring-up firmware.
  void drawFirmwareInfo(const char *projectName, const char *firmwareVersion);

  bool isReady() const;
  uint8_t address() const;

  // Accesso al driver Adafruit per renderer piu alti, come MenuController.
  Adafruit_SSD1306 &display();

private:
  bool i2cDeviceResponds(uint8_t address);
  void logConfig(const char *label);

  OledMonitorConfig _config;
  LoggerInterface &_logger;
  Adafruit_SSD1306 _display;
  bool _ready;
  uint8_t _address;
};
