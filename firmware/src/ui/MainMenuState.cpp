#include "MainMenuState.h"

const char *const MainMenuState::ITEMS[MainMenuState::ITEM_COUNT] = {
  "Mode",
  "Informazioni"
};

MainMenuState::MainMenuState() : _selectedIndex(0) {
}

void MainMenuState::moveNext() {
  if (hasNext()) {
    _selectedIndex++;
  }
}

void MainMenuState::movePrevious() {
  if (hasPrevious()) {
    _selectedIndex--;
  }
}

void MainMenuState::reset() {
  _selectedIndex = 0;
}

uint8_t MainMenuState::selectedIndex() const {
  return _selectedIndex;
}

const char *MainMenuState::currentLabel() const {
  return ITEMS[_selectedIndex];
}

bool MainMenuState::hasPrevious() const {
  return _selectedIndex > 0;
}

bool MainMenuState::hasNext() const {
  return _selectedIndex + 1 < ITEM_COUNT;
}
