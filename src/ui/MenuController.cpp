#include "MenuController.h"

namespace {
extern const MenuController::MenuPage HOME_PAGE;

const MenuController::MenuItem MODE_ITEMS[] = {
  {"Meditazione", nullptr, MenuController::MENU_ACTION_SELECT_ACTIVITY, "Meditazione", "meditazione"},
  {"Lettura", nullptr, MenuController::MENU_ACTION_SELECT_ACTIVITY, "Lettura", "lettura"},
  {"Videogame", nullptr, MenuController::MENU_ACTION_SELECT_ACTIVITY, "Videogame", "videogame"}
};

const MenuController::MenuPage MODE_PAGE = {
  "Modalita",
  &HOME_PAGE,
  MODE_ITEMS,
  sizeof(MODE_ITEMS) / sizeof(MODE_ITEMS[0])
};

const MenuController::MenuItem HOME_ITEMS[] = {
  {"", nullptr, MenuController::MENU_ACTION_TOGGLE_ACTIVITY, nullptr, nullptr},
  {"Modalita", &MODE_PAGE, MenuController::MENU_ACTION_NONE, nullptr, nullptr}
};

const MenuController::MenuPage HOME_PAGE = {
  "Home",
  nullptr,
  HOME_ITEMS,
  sizeof(HOME_ITEMS) / sizeof(HOME_ITEMS[0])
};

MenuEvent noEvent() {
  return {MENU_EVENT_NONE, nullptr, nullptr};
}
}

MenuController::MenuController(const MenuControllerConfig &config, OledMonitor &oled, LoggerInterface &logger)
    : _config(config),
      _oled(oled),
      _logger(logger),
      _currentPage(&HOME_PAGE),
      _selectedIndex(0),
      _selectedActivityLabel("Meditazione"),
      _selectedActivityKey("meditazione"),
      _activityRunning(false),
      _wifiState(WIFI_UI_DISCONNECTED),
      _lastWifiStartupDrawAt(0) {
}

MenuEvent MenuController::handleInput(MenuInput input) {
  MenuEvent event = noEvent();

  switch (input) {
    case MENU_INPUT_PLUS:
      moveNext();
      break;
    case MENU_INPUT_MINUS:
      movePrevious();
      break;
    case MENU_INPUT_SELECT:
      event = selectCurrentItem();
      break;
    case MENU_INPUT_CANCEL:
      cancel();
      break;
  }

  logEvent(event);
  draw();
  return event;
}

void MenuController::draw() {
  if (!_oled.isReady()) {
    return;
  }

  drawFrame();

  _oled.display().setTextSize(1);
  _oled.display().setCursor(0, 16);

  if (_currentPage == &HOME_PAGE) {
    _oled.display().print(_activityRunning ? "Avviata:" : "Attivita:");
    _oled.display().setCursor(58, 16);
    _oled.display().print(_selectedActivityLabel);
  } else {
    _oled.display().print(_currentPage->title);
  }

  drawCenteredText(currentItemLabel(), 31, 2);
  _oled.display().display();
}

void MenuController::drawWifiStartupScreen(const char *ssid, unsigned long elapsedMs, unsigned long timeoutMs) {
  if (!_oled.isReady()) {
    return;
  }

  if (elapsedMs >= _lastWifiStartupDrawAt &&
      elapsedMs - _lastWifiStartupDrawAt < _config.wifiAnimationIntervalMs) {
    return;
  }
  _lastWifiStartupDrawAt = elapsedMs;

  drawFrame();

  _oled.display().setTextSize(1);
  _oled.display().setCursor(0, 18);
  _oled.display().print("Connessione WiFi");

  _oled.display().setCursor(0, 30);
  _oled.display().print(ssid);

  _oled.display().setCursor(0, 42);
  _oled.display().print("Timeout ");
  _oled.display().print((timeoutMs - elapsedMs) / 1000);
  _oled.display().print("s");

  _oled.display().display();
}

void MenuController::setWifiState(WifiUiState wifiState) {
  _wifiState = wifiState;
}

bool MenuController::isActivityRunning() const {
  return _activityRunning;
}

const char *MenuController::selectedActivityLabel() const {
  return _selectedActivityLabel;
}

const char *MenuController::selectedActivityKey() const {
  return _selectedActivityKey;
}

void MenuController::moveNext() {
  _selectedIndex = (_selectedIndex + 1) % _currentPage->itemCount;
}

void MenuController::movePrevious() {
  if (_selectedIndex == 0) {
    _selectedIndex = _currentPage->itemCount - 1;
  } else {
    _selectedIndex--;
  }
}

MenuEvent MenuController::selectCurrentItem() {
  const MenuItem &item = currentItem();

  if (item.childPage != nullptr) {
    goToPage(item.childPage);
    return noEvent();
  }

  switch (item.action) {
    case MENU_ACTION_TOGGLE_ACTIVITY:
      _activityRunning = !_activityRunning;
      return {
        _activityRunning ? MENU_EVENT_ACTIVITY_STARTED : MENU_EVENT_ACTIVITY_STOPPED,
        _selectedActivityLabel,
        _selectedActivityKey
      };
    case MENU_ACTION_SELECT_ACTIVITY:
      _selectedActivityLabel = item.activityLabel;
      _selectedActivityKey = item.activityKey;
      goToPage(&HOME_PAGE);
      return {MENU_EVENT_ACTIVITY_CHANGED, _selectedActivityLabel, _selectedActivityKey};
    case MENU_ACTION_NONE:
      break;
  }

  return noEvent();
}

void MenuController::cancel() {
  if (_currentPage->parentPage != nullptr) {
    goToPage(_currentPage->parentPage);
  } else {
    goToPage(&HOME_PAGE);
  }
}

void MenuController::goToPage(const MenuPage *page) {
  _currentPage = page;
  _selectedIndex = 0;
}

const MenuController::MenuItem &MenuController::currentItem() const {
  return _currentPage->items[_selectedIndex];
}

const char *MenuController::currentItemLabel() const {
  const MenuItem &item = currentItem();
  if (item.action == MENU_ACTION_TOGGLE_ACTIVITY) {
    return _activityRunning ? "Stop" : "Start";
  }

  return item.label;
}

void MenuController::drawCenteredText(const char *text, int16_t y, uint8_t textSize) {
  int16_t x1;
  int16_t y1;
  uint16_t w;
  uint16_t h;

  _oled.display().setTextSize(textSize);
  _oled.display().getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  _oled.display().setCursor((_config.width - w) / 2, y);
  _oled.display().print(text);
}

void MenuController::drawFrame() {
  _oled.display().clearDisplay();
  _oled.display().setTextColor(SSD1306_WHITE);
  drawHeader();
  _oled.display().drawLine(0, _config.headerBottomY, _config.width - 1, _config.headerBottomY, SSD1306_WHITE);
  _oled.display().drawLine(0, _config.footerTopY, _config.width - 1, _config.footerTopY, SSD1306_WHITE);

  _oled.display().setTextSize(1);
  _oled.display().setCursor(0, 56);
  _oled.display().print("Mode: ");
  _oled.display().print(_selectedActivityLabel);
}

void MenuController::drawHeader() {
  _oled.display().setTextSize(1);
  _oled.display().setTextColor(SSD1306_WHITE);
  _oled.display().setCursor(0, 1);
  _oled.display().print("EEG Monitor");
  _oled.display().drawLine(74, 0, 74, 9, SSD1306_WHITE);
  drawWifiIcon(88, 1);
}

void MenuController::drawWifiIcon(int16_t x, int16_t y) {
  switch (_wifiState) {
    case WIFI_UI_CONNECTED:
      drawWifiConnectedIcon(x, y);
      break;
    case WIFI_UI_CONNECTING:
      drawWifiConnectingIcon(x, y);
      break;
    case WIFI_UI_DISCONNECTED:
      drawWifiDisconnectedIcon(x, y);
      break;
  }
}

void MenuController::drawWifiDisconnectedIcon(int16_t x, int16_t y) {
  _oled.display().drawRect(x, y + 7, 3, 3, SSD1306_WHITE);
  _oled.display().drawRect(x + 5, y + 4, 3, 6, SSD1306_WHITE);
  _oled.display().drawRect(x + 10, y + 1, 3, 9, SSD1306_WHITE);
  _oled.display().drawLine(x - 1, y, x + 15, y + 10, SSD1306_WHITE);
}

void MenuController::drawWifiConnectedIcon(int16_t x, int16_t y) {
  _oled.display().fillRect(x, y + 7, 3, 3, SSD1306_WHITE);
  _oled.display().fillRect(x + 5, y + 4, 3, 6, SSD1306_WHITE);
  _oled.display().fillRect(x + 10, y + 1, 3, 9, SSD1306_WHITE);
}

void MenuController::drawWifiConnectingIcon(int16_t x, int16_t y) {
  const uint8_t step = (millis() / _config.wifiAnimationIntervalMs) % 4;

  _oled.display().drawRect(x, y + 7, 3, 3, SSD1306_WHITE);
  _oled.display().drawRect(x + 5, y + 4, 3, 6, SSD1306_WHITE);
  _oled.display().drawRect(x + 10, y + 1, 3, 9, SSD1306_WHITE);

  if (step >= 1) {
    _oled.display().fillRect(x, y + 7, 3, 3, SSD1306_WHITE);
  }
  if (step >= 2) {
    _oled.display().fillRect(x + 5, y + 4, 3, 6, SSD1306_WHITE);
  }
  if (step >= 3) {
    _oled.display().fillRect(x + 10, y + 1, 3, 9, SSD1306_WHITE);
  }
}

void MenuController::logEvent(const MenuEvent &event) {
  if (!_config.loggingEnabled || event.type == MENU_EVENT_NONE) {
    return;
  }

  _logger.print(F("MENU | evento "));
  switch (event.type) {
    case MENU_EVENT_ACTIVITY_STARTED:
      _logger.print(F("activity started"));
      break;
    case MENU_EVENT_ACTIVITY_STOPPED:
      _logger.print(F("activity stopped"));
      break;
    case MENU_EVENT_ACTIVITY_CHANGED:
      _logger.print(F("activity changed"));
      break;
    case MENU_EVENT_NONE:
      break;
  }
  _logger.print(F(" | activity: "));
  _logger.println(event.activityLabel);
}
