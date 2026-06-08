#include <unity.h>

#include "../../src/ui/MainMenuState.h"
#include "../../src/ui/MainMenuState.cpp"

void setUp() {
}

void tearDown() {
}

void test_main_menu_starts_on_mode_with_only_next_available() {
  MainMenuState menu;

  TEST_ASSERT_EQUAL_UINT8(0, menu.selectedIndex());
  TEST_ASSERT_EQUAL_STRING("Mode", menu.currentLabel());
  TEST_ASSERT_FALSE(menu.hasPrevious());
  TEST_ASSERT_TRUE(menu.hasNext());
}

void test_plus_moves_to_informazioni_and_stops_at_last_item() {
  MainMenuState menu;

  menu.moveNext();
  TEST_ASSERT_EQUAL_UINT8(1, menu.selectedIndex());
  TEST_ASSERT_EQUAL_STRING("Informazioni", menu.currentLabel());
  TEST_ASSERT_TRUE(menu.hasPrevious());
  TEST_ASSERT_FALSE(menu.hasNext());

  menu.moveNext();
  TEST_ASSERT_EQUAL_UINT8(1, menu.selectedIndex());
  TEST_ASSERT_EQUAL_STRING("Informazioni", menu.currentLabel());
}

void test_minus_moves_back_to_mode_and_stops_at_first_item() {
  MainMenuState menu;

  menu.moveNext();
  menu.movePrevious();
  TEST_ASSERT_EQUAL_UINT8(0, menu.selectedIndex());
  TEST_ASSERT_EQUAL_STRING("Mode", menu.currentLabel());
  TEST_ASSERT_FALSE(menu.hasPrevious());
  TEST_ASSERT_TRUE(menu.hasNext());

  menu.movePrevious();
  TEST_ASSERT_EQUAL_UINT8(0, menu.selectedIndex());
  TEST_ASSERT_EQUAL_STRING("Mode", menu.currentLabel());
}

void test_reset_returns_to_mode() {
  MainMenuState menu;

  menu.moveNext();
  menu.reset();

  TEST_ASSERT_EQUAL_UINT8(0, menu.selectedIndex());
  TEST_ASSERT_EQUAL_STRING("Mode", menu.currentLabel());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_main_menu_starts_on_mode_with_only_next_available);
  RUN_TEST(test_plus_moves_to_informazioni_and_stops_at_last_item);
  RUN_TEST(test_minus_moves_back_to_mode_and_stops_at_first_item);
  RUN_TEST(test_reset_returns_to_mode);
  return UNITY_END();
}
