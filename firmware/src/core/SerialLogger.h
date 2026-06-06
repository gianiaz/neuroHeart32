#pragma once

#include "LoggerInterface.h"

class SerialLogger : public LoggerInterface {
public:
  void print(const char *message) override {
    Serial.print(message);
  }

  void print(const __FlashStringHelper *message) override {
    Serial.print(message);
  }

  void print(int value) override {
    Serial.print(value);
  }

  void print(unsigned long value) override {
    Serial.print(value);
  }

  void print(uint8_t value, int base) override {
    Serial.print(value, base);
  }

  void println() override {
    Serial.println();
  }

  void println(const char *message) override {
    Serial.println(message);
  }

  void println(const __FlashStringHelper *message) override {
    Serial.println(message);
  }

  void println(int value) override {
    Serial.println(value);
  }

  void println(unsigned long value) override {
    Serial.println(value);
  }

  void println(uint8_t value, int base) override {
    Serial.println(value, base);
  }
};
