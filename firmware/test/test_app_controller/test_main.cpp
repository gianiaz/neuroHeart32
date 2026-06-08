#include <string>
#include <vector>

#include <unity.h>

#include "../../src/app/AppController.h"
#include "../../src/app/AppController.cpp"

class FakeLogger : public AppLogger {
public:
  std::vector<std::string> events;

  void mainInitializationStarted() override {
    events.push_back("log:init");
  }

  void mainFirmwareInfo(const char *projectName, const char *firmwareVersion) override {
    events.push_back(std::string("log:firmware:") + projectName + ":" + firmwareVersion);
  }

  void mainOledInitialized(uint8_t sdaPin, uint8_t sclPin, uint8_t i2cAddress) override {
    events.push_back("log:oled-ready");
    lastSdaPin = sdaPin;
    lastSclPin = sclPin;
    lastI2cAddress = i2cAddress;
  }

  void mainOledInitializationFailed() override {
    events.push_back("log:oled-failed");
  }

  void mainDelayedStartupReport(const char *projectName,
                                const char *firmwareVersion,
                                uint8_t sdaPin,
                                uint8_t sclPin,
                                bool oledReady) override {
    events.push_back(std::string("log:delayed:") + projectName + ":" + firmwareVersion);
    lastSdaPin = sdaPin;
    lastSclPin = sclPin;
    lastDelayedOledReady = oledReady;
  }

  uint8_t lastSdaPin = 0;
  uint8_t lastSclPin = 0;
  uint8_t lastI2cAddress = 0;
  bool lastDelayedOledReady = false;
};

class FakeOledMonitor : public AppOledMonitor {
public:
  explicit FakeOledMonitor(bool beginResult) : beginResult(beginResult) {
  }

  bool begin() override {
    calls.push_back("oled:begin");
    return beginResult;
  }

  void clear() override {
    calls.push_back("oled:clear");
  }

  void drawFirmwareInfo(const char *projectName, const char *firmwareVersion) override {
    calls.push_back(std::string("oled:draw:") + projectName + ":" + firmwareVersion);
  }

  void printStatus() override {
    calls.push_back("oled:status");
  }

  uint8_t sdaPin() const override {
    return 5;
  }

  uint8_t sclPin() const override {
    return 6;
  }

  uint8_t i2cAddress() const override {
    return 0x3C;
  }

  bool beginResult;
  std::vector<std::string> calls;
};

void setUp() {
}

void tearDown() {
}

AppControllerConfig testConfig(bool mainLoggingEnabled = true) {
  return {
    "NeuroHeart32",
    "0.0.2",
    2000,
    {
      mainLoggingEnabled, // main
      false,              // oled
      false,              // wifi
      false,              // wifiScan
      false,              // mqtt
      false,              // menu
      false               // eeg
    }
  };
}

void test_setup_initializes_oled_and_draws_firmware_when_oled_begin_succeeds() {
  FakeOledMonitor oled(true);
  FakeLogger logger;
  AppController app(testConfig(), oled, logger);

  app.setup();

  TEST_ASSERT_TRUE(app.oledReady());
  TEST_ASSERT_EQUAL_STRING("log:init", logger.events[0].c_str());
  TEST_ASSERT_EQUAL_STRING("log:firmware:NeuroHeart32:0.0.2", logger.events[1].c_str());
  TEST_ASSERT_EQUAL_STRING("log:oled-ready", logger.events[2].c_str());
  TEST_ASSERT_EQUAL_UINT8(5, logger.lastSdaPin);
  TEST_ASSERT_EQUAL_UINT8(6, logger.lastSclPin);
  TEST_ASSERT_EQUAL_UINT8(0x3C, logger.lastI2cAddress);

  TEST_ASSERT_EQUAL_STRING("oled:begin", oled.calls[0].c_str());
  TEST_ASSERT_EQUAL_STRING("oled:clear", oled.calls[1].c_str());
  TEST_ASSERT_EQUAL_STRING("oled:draw:NeuroHeart32:0.0.2", oled.calls[2].c_str());
  TEST_ASSERT_EQUAL_STRING("oled:status", oled.calls[3].c_str());
}

void test_setup_does_not_draw_firmware_when_oled_begin_fails() {
  FakeOledMonitor oled(false);
  FakeLogger logger;
  AppController app(testConfig(), oled, logger);

  app.setup();

  TEST_ASSERT_FALSE(app.oledReady());
  TEST_ASSERT_EQUAL_STRING("log:init", logger.events[0].c_str());
  TEST_ASSERT_EQUAL_STRING("log:oled-failed", logger.events[1].c_str());
  TEST_ASSERT_EQUAL(1, oled.calls.size());
  TEST_ASSERT_EQUAL_STRING("oled:begin", oled.calls[0].c_str());
}

void test_delayed_startup_report_is_printed_once_after_configured_delay() {
  FakeOledMonitor oled(true);
  FakeLogger logger;
  AppController app(testConfig(), oled, logger);

  app.setup();
  app.loop(1999);
  TEST_ASSERT_FALSE(app.startupReportPrinted());

  app.loop(2000);
  TEST_ASSERT_TRUE(app.startupReportPrinted());
  TEST_ASSERT_EQUAL_STRING("log:delayed:NeuroHeart32:0.0.2", logger.events.back().c_str());
  TEST_ASSERT_TRUE(logger.lastDelayedOledReady);

  const size_t eventCount = logger.events.size();
  app.loop(5000);
  TEST_ASSERT_EQUAL(eventCount, logger.events.size());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_setup_initializes_oled_and_draws_firmware_when_oled_begin_succeeds);
  RUN_TEST(test_setup_does_not_draw_firmware_when_oled_begin_fails);
  RUN_TEST(test_delayed_startup_report_is_printed_once_after_configured_delay);
  return UNITY_END();
}
