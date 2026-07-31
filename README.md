# VitaDoctor Hardware Diagnostic v1.01

VitaDoctor is a complete hardware testing and diagnostic utility for the Sony PlayStation Vita (PCH-1000 OLED, PCH-2000 Slim, PSTV / VTE-1000, PTEL-1000 Testing Kit, and PDEL-1000 Development Tool).

It runs directly as a standalone VPK application without requiring background kernel plugins.

---

## What's New in Version 1.01

- Added hardware detection for PS Vita TEST (PTEL-1000) and DevKit (PDEL-1000) units.
- Integrated dynamic model recognition (PCH-1000 OLED vs PCH-2000 Slim vs PSTV).
- Added Motherboard Revision detection (IRS-002 / PCA-001, DOL-1001, VTE-1000).
- Added Analog Stick Circularity Error % calculator.
- Added interactive PS Button Blue LED pulse test (Triangle / Circle controls).
- Added Camera hardware viewfinder test with Square port switcher and Triangle stop button.
- Updated total system audit report suite with Pass/Fail health scoring and actionable repair advice.

---

## Diagnostic Suite Tabs

1. **[1] STICKS**: Visual position indicators for Left/Right analog sticks with raw values (`0-255`), offset %, circularity error %, button matrix test (D-Pad, Face Buttons, Triggers, Select, Start), and Touch panel test.
2. **[2] BATTERY**: Real-time battery %, cell health estimate (2210 mAh base), voltage (mV), temperature (°C/°F), remaining time, and 60-second voltage history graph.
3. **[3] DISPLAY**: Fullscreen RGB, White, Black, Grayscale, and Grid color test patterns, plus built-in OLED Exerciser to prevent burn-in.
4. **[4] MOTION**: 3D Artificial horizon gauge, Gyroscope (rad/s) and Accelerometer ($g$) sensor axes readout.
5. **[5] AUDIO**: Separate Left (440 Hz) and Right (880 Hz) stereo speaker test tones and live Microphone graphical VU-Meter.
6. **[6] CAMERA**: Dual Camera hardware detection (Front/Rear OV2720 sensors), Square port switcher, and Triangle stop button.
7. **[7] SYSTEM**: Console model detection (PCH-1000 OLED, PCH-2000 Slim, PSTV, PTEL-1000, PDEL-1000), Wi-Fi IP address, hardware MAC address, PS Button LED pulse test, and partition monitoring (`ux0:`, `ur0:`, `uma0:`, `gro0:`).
8. **[8] REPORT**: Interactive & customizable diagnostic test suite. Select individual tests with D-Pad/Cross, press Square to select all, and press START to run total system audit with Pass/Fail score calculation.

---

## Controls

- **SEL + L / R**: Switch between diagnostic tabs
- **SEL + START**: Exit VitaDoctor
- **Square / Circle** *(Audio Tab)*: Play Left / Right speaker test tone
- **Square** *(Display Tab)*: Toggle OLED Exerciser
- **Square / Triangle** *(Camera Tab)*: Switch Front / Rear Camera or Stop test
- **Triangle / Circle** *(System Tab)*: Pulse PS Button Blue LED / Turn Off
- **D-Pad Up / Down, Cross, Square** *(Report Tab)*: Navigate & toggle diagnostic tests
- **START** *(Report Tab)*: Run Total System Audit

---

## Building from Source

VitaDoctor is built using the standard VitaSDK toolchain.

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake
make
```

Output VPK will be generated in `build/VitaDoctor.vpk`.

---

## Credits & Special Thanks

- **VitaSDK Developers**: For creating and maintaining the open-source PS Vita SDK, headers, and toolchain.
- **TheFloW**: For his legendary work, documentation, and tools for the PS Vita scene.
- **xerpi**: For `vita2dlib`.

---

## License

GPL-3.0 License.
