#pragma once

#include <Arduino.h>

#include "../connectivity/WifiManager.h"
#include "../core/LoggerInterface.h"
#include "../devices/OledMonitor.h"

enum MenuInput {
  MENU_INPUT_PLUS,
  MENU_INPUT_MINUS,
  MENU_INPUT_SELECT,
  MENU_INPUT_CANCEL
};

enum MenuEventType {
  MENU_EVENT_NONE,
  MENU_EVENT_ACTIVITY_STARTED,
  MENU_EVENT_ACTIVITY_STOPPED,
  MENU_EVENT_ACTIVITY_CHANGED
};

struct MenuEvent {
  MenuEventType type;
  const char *activityLabel;
  const char *activityKey;
};

struct MenuControllerConfig {
  uint8_t width;
  uint8_t height;
  uint8_t headerBottomY;
  uint8_t footerTopY;
  unsigned long wifiAnimationIntervalMs;
  bool loggingEnabled;
};

class MenuController {
public:
  enum MenuAction {
    MENU_ACTION_NONE,
    MENU_ACTION_TOGGLE_ACTIVITY,
    MENU_ACTION_SELECT_ACTIVITY
  };

  struct MenuPage;

  struct MenuItem {
    const char *label;
    const MenuPage *childPage;
    MenuAction action;
    const char *activityLabel;
    const char *activityKey;
  };

  struct MenuPage {
    const char *title;
    const MenuPage *parentPage;
    const MenuItem *items;
    uint8_t itemCount;
  };

  MenuController(const MenuControllerConfig &config, OledMonitor &oled, LoggerInterface &logger);

  MenuEvent handleInput(MenuInput input);
  void draw();
  void drawWifiStartupScreen(const char *ssid, unsigned long elapsedMs, unsigned long timeoutMs);
  void setWifiState(WifiUiState wifiState);

  bool isActivityRunning() const;
  const char *selectedActivityLabel() const;
  const char *selectedActivityKey() const;

private:
  void moveNext();
  void movePrevious();
  MenuEvent selectCurrentItem();
  void cancel();
  void goToPage(const MenuPage *page);

  const MenuItem &currentItem() const;
  const char *currentItemLabel() const;
  void drawCenteredText(const char *text, int16_t y, uint8_t textSize);
  void drawFrame();
  void drawHeader();
  void drawWifiIcon(int16_t x, int16_t y);
  void drawWifiDisconnectedIcon(int16_t x, int16_t y);
  void drawWifiConnectedIcon(int16_t x, int16_t y);
  void drawWifiConnectingIcon(int16_t x, int16_t y);
  void logEvent(const MenuEvent &event);

  MenuControllerConfig _config;
  OledMonitor &_oled;
  LoggerInterface &_logger;
  const MenuPage *_currentPage;
  uint8_t _selectedIndex;
  const char *_selectedActivityLabel;
  const char *_selectedActivityKey;
  bool _activityRunning;
  WifiUiState _wifiState;
  unsigned long _lastWifiStartupDrawAt;
};
