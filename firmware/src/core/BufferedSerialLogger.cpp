#include "BufferedSerialLogger.h"

BufferedSerialLogger::BufferedSerialLogger(size_t capacity)
    : _buffer(new char[capacity]),
      _capacity(capacity),
      _head(0),
      _tail(0),
      _full(false),
      _begun(false) {
}

BufferedSerialLogger::~BufferedSerialLogger() {
  delete[] _buffer;
}

void BufferedSerialLogger::begin(unsigned long baudRate) {
  if (_begun) {
    return;
  }

  Serial.begin(baudRate);
  _begun = true;
  flush();
}

void BufferedSerialLogger::flush() {
  if (!serialReady()) {
    return;
  }

  writeBufferedToSerial();
}

void BufferedSerialLogger::forceFlush() {
  if (!_begun) {
    return;
  }

  writeBufferedToSerial();
}

void BufferedSerialLogger::writeBufferedToSerial() {
  while (_tail != _head || _full) {
    Serial.write(_buffer[_tail]);
    _tail = (_tail + 1) % _capacity;
    _full = false;
  }
}

bool BufferedSerialLogger::isConnected() const {
  return serialReady();
}

void BufferedSerialLogger::print(const char *message) {
  writeText(message);
}

void BufferedSerialLogger::print(const __FlashStringHelper *message) {
  writeText(reinterpret_cast<const char *>(message));
}

void BufferedSerialLogger::print(int value) {
  writeNumber(value);
}

void BufferedSerialLogger::print(unsigned long value) {
  writeUnsignedNumber(value);
}

void BufferedSerialLogger::print(uint8_t value, int base) {
  writeUnsignedNumber(value, base);
}

void BufferedSerialLogger::println() {
  writeText("\r\n");
}

void BufferedSerialLogger::println(const char *message) {
  print(message);
  println();
}

void BufferedSerialLogger::println(const __FlashStringHelper *message) {
  print(message);
  println();
}

void BufferedSerialLogger::println(int value) {
  print(value);
  println();
}

void BufferedSerialLogger::println(unsigned long value) {
  print(value);
  println();
}

void BufferedSerialLogger::println(uint8_t value, int base) {
  print(value, base);
  println();
}

void BufferedSerialLogger::writeChar(char value) {
  if (serialReady()) {
    flush();
    Serial.write(value);
    return;
  }

  bufferChar(value);
}

void BufferedSerialLogger::writeText(const char *message) {
  if (message == nullptr) {
    return;
  }

  while (*message != '\0') {
    writeChar(*message);
    message++;
  }
}

void BufferedSerialLogger::writeNumber(long value, uint8_t base) {
  if (value < 0 && base == 10) {
    writeChar('-');
    writeUnsignedNumber((unsigned long)(-value), base);
    return;
  }

  writeUnsignedNumber((unsigned long)value, base);
}

void BufferedSerialLogger::writeUnsignedNumber(unsigned long value, uint8_t base) {
  char digits[33];
  uint8_t index = 0;

  if (base < 2) {
    base = 10;
  }

  do {
    const uint8_t digit = value % base;
    digits[index++] = digit < 10 ? '0' + digit : 'A' + (digit - 10);
    value /= base;
  } while (value > 0 && index < sizeof(digits));

  while (index > 0) {
    writeChar(digits[--index]);
  }
}

void BufferedSerialLogger::bufferChar(char value) {
  if (_capacity == 0) {
    return;
  }

  _buffer[_head] = value;
  _head = (_head + 1) % _capacity;

  if (_full) {
    _tail = (_tail + 1) % _capacity;
  }

  _full = _head == _tail;
}

bool BufferedSerialLogger::serialReady() const {
  return _begun && Serial;
}
