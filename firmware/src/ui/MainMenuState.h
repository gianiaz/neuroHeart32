#pragma once

#include <stdint.h>

class MainMenuState {
public:
  MainMenuState();

  void moveNext();
  void movePrevious();
  void reset();

  uint8_t selectedIndex() const;
  const char *currentLabel() const;
  bool hasPrevious() const;
  bool hasNext() const;

private:
  static constexpr uint8_t ITEM_COUNT = 2;
  static const char *const ITEMS[ITEM_COUNT];

  uint8_t _selectedIndex;
};
