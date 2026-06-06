#pragma once

#include <Arduino.h>

class LoggerInterface {
public:
  virtual ~LoggerInterface() = default;

  virtual void print(const char *message) = 0;
  virtual void print(const __FlashStringHelper *message) = 0;
  virtual void print(int value) = 0;
  virtual void print(unsigned long value) = 0;
  virtual void print(uint8_t value, int base) = 0;

  virtual void println() = 0;
  virtual void println(const char *message) = 0;
  virtual void println(const __FlashStringHelper *message) = 0;
  virtual void println(int value) = 0;
  virtual void println(unsigned long value) = 0;
  virtual void println(uint8_t value, int base) = 0;
};
