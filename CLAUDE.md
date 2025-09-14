# Claude Code Notes

## Project Status
- **Hardware**: LilyGo T5 4.7" E-Paper (ESP32-WROVER-E, 4MB Flash)
- **Touch**: Capacitive overlay (I2C: SDA=15, SCL=14, INT=13)
- **Current Features**: HA Dashboard, Touch controls, State tracking

## Code Structure (Refactored)
```
src/
├── main.cpp              (needs cleanup)
├── common/
│   └── definitions.h     (shared types, constants)
├── modules/
│   ├── ha_api.h/.cpp     (HA API calls, state tracking)
│   ├── display_ui.h/.cpp (UI rendering - partial)
│   └── touch_handler.h/.cpp (touch input, zones)
└── configurations.h      (user config)
```

## Touch System
- **Pins**: SDA=15, SCL=14 (not 21,22!)
- **State Machine**: touchActive prevents double-triggers
- **Coordinate Fix**: y = EPD_HEIGHT - y (vertical flip)
- **Update Policy**: No auto screen refresh after toggle

## Current Issues
- main.cpp still contains all display functions (not fully modularized)
- Touch works but display_ui.cpp incomplete

## Next Steps
1. Complete display_ui.cpp extraction
2. Clean up main.cpp imports
3. Add page system
4. Test modular structure

## Git Setup
- origin: DasPflanze fork
- upstream: CybDis original

## Commit Guidelines
- No Claude mentions in commits
- Focus on what was changed, not who did it
- Keep technical and concise