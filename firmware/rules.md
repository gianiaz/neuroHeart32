# Project Rules

## Repository Context

- Working directory: `C:\Gianiaz\NeuroHeart32\firmware`.
- PlatformIO firmware project for an ESP32-C3 SuperMini style board.
- Main PlatformIO environment: `esp32-c3-supermini`.
- Native test environment: `native`.

## Architecture Notes

- `src/main.cpp` owns hardware/application wiring:
  - OLED constants and config.
  - firmware/project constants:
    - `PROJECT_NAME`
    - `FIRMWARE_VERSION`
  - logger setup.
  - `OledMonitorAppAdapter`.
  - `AppController app(...)`.
- `src/app/AppController.*` owns the startup flow:
  - initializes OLED via `AppOledMonitor`.
  - logs startup status.
  - draws the initial OLED screen through `drawFirmwareInfo(...)`.
  - emits the delayed serial startup report.
- `src/devices/OledMonitor.*` owns direct SSD1306/I2C display access:
  - configures I2C pins.
  - initializes the Adafruit SSD1306 driver.
  - renders low-level startup/diagnostic screens.
  - exposes `Adafruit_SSD1306 &display()` for higher-level UI.
- `src/ui/MenuController.*` owns menu rendering/navigation logic:
  - uses `OledMonitor::display()` to draw UI.
  - current main menu direction is horizontal.
  - main menu entries are `Mode` and `Informazioni`.
  - `MENU_INPUT_PLUS` moves right only if another item exists.
  - `MENU_INPUT_MINUS` moves left only if a previous item exists.
- `src/ui/MainMenuState.*` owns testable main-menu navigation state.
- `src/assets/LogoBitmap.h` stores the OLED logo bitmap for future use.

## OLED/UI Rules

- Display is SSD1306 `128x64`.
- OLED library stack:
  - `adafruit/Adafruit SSD1306`
  - `adafruit/Adafruit GFX Library`
- Use ASCII display text unless a custom font/encoding strategy is added.
  - Prefer ASCII-only labels on the default Adafruit font.
- Startup screen currently shows:
  - first row: `PROJECT_NAME`
  - second row: `Version: FIRMWARE_VERSION`
  - third row: horizontal separator line
  - centered menu item `Mode`
  - right triangle when another menu item exists.
- Keep firmware version in one constant: `FIRMWARE_VERSION` in `src/main.cpp`.
- The logo should remain available in `src/assets/LogoBitmap.h`, but not shown on startup unless explicitly requested.

## Build And Test Commands

Build the firmware for the board:

```powershell
C:\Users\giani\.platformio\penv\Scripts\pio.exe run -e esp32-c3-supermini
```

Run native tests on Windows with MSYS toolchain in PATH:

```powershell
$env:Path = 'C:\msys64\ucrt64\bin;C:\msys64\mingw64\bin;C:\msys64\usr\bin;' + $env:Path
& C:\Users\giani\.platformio\penv\Scripts\pio.exe test -e native
```

Do not rely on plain `pio run` for final verification unless the native compiler is already available in PATH. Plain `pio run` builds both environments and may fail on `native` with `gcc`/`g++` not found even when the ESP32-C3 firmware build succeeds.

## Test Notes

- Native unit tests live under `test/`.
- `test/test_app_controller/test_main.cpp` verifies startup controller flow.
- `test/test_log_config/test_main.cpp` verifies logging channel behavior.
- `test/test_main_menu_state/test_main.cpp` verifies main-menu navigation boundaries and labels.
- If changing startup flow or project/version constants, update app controller tests accordingly.

## Git/Workspace Notes

- There may be unrelated dirty files outside `firmware/`, especially under schematics or generated IDE folders.
- Do not revert unrelated user changes.
- Use focused diffs for files touched by the current task.
