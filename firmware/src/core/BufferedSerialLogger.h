#pragma once

#include "LoggerInterface.h"

class BufferedSerialLogger : public LoggerInterface {
public:
  explicit BufferedSerialLogger(size_t capacity = 1024);
  ~BufferedSerialLogger();

  void begin(unsigned long baudRate);
  void flush();
  void forceFlush();
  bool isConnected() const;

  void print(const char *message) override;
  void print(const __FlashStringHelper *message) override;
  void print(int value) override;
  void print(unsigned long value) override;
  void print(uint8_t value, int base) override;

  void println() override;
  void println(const char *message) override;
  void println(const __FlashStringHelper *message) override;
  void println(int value) override;
  void println(unsigned long value) override;
  void println(uint8_t value, int base) override;

private:
  void writeChar(char value);
  void writeText(const char *message);
  void writeNumber(long value, uint8_t base = 10);
  void writeUnsignedNumber(unsigned long value, uint8_t base = 10);
  void bufferChar(char value);
  bool serialReady() const;
  void writeBufferedToSerial();

  char *_buffer;
  size_t _capacity;
  size_t _head;
  size_t _tail;
  bool _full;
  bool _begun;
};
