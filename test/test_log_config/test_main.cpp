#include <unity.h>

#include "../../src/config/LogConfig.h"

void setUp() {
}

void tearDown() {
}

void test_serial_logging_is_disabled_when_all_channels_are_disabled() {
  const LogConfig config = {
    false, // main
    false, // oled
    false, // wifi
    false, // wifiScan
    false, // mqtt
    false, // menu
    false  // eeg
  };

  TEST_ASSERT_FALSE(isSerialLoggingEnabled(config));
}

void test_serial_logging_is_enabled_when_main_channel_is_enabled() {
  const LogConfig config = {
    true,  // main
    false, // oled
    false, // wifi
    false, // wifiScan
    false, // mqtt
    false, // menu
    false  // eeg
  };

  TEST_ASSERT_TRUE(isSerialLoggingEnabled(config));
}

void test_serial_logging_is_enabled_when_any_module_channel_is_enabled() {
  const LogConfig config = {
    false, // main
    false, // oled
    false, // wifi
    false, // wifiScan
    true,  // mqtt
    false, // menu
    false  // eeg
  };

  TEST_ASSERT_TRUE(isSerialLoggingEnabled(config));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_serial_logging_is_disabled_when_all_channels_are_disabled);
  RUN_TEST(test_serial_logging_is_enabled_when_main_channel_is_enabled);
  RUN_TEST(test_serial_logging_is_enabled_when_any_module_channel_is_enabled);
  return UNITY_END();
}
