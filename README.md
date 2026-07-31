# VitaDoctor Hardware Diagnostic

VitaDoctor is a standalone hardware testing and diagnostic tool for the PlayStation Vita (PCH-1000 OLED, PCH-2000 Slim, and PlayStation TV / VTE-1000).

It runs directly as a VPK application without requiring background plugins.

---

## Features

- **Analog Sticks & Buttons Test**
  - Live visual position indicators for Left and Right analog sticks with raw values (`0-255`) and percentage offset calculations.
  - Complete button matrix test for D-Pad, Face Buttons, Shoulder Triggers (`L`/`R`), `SELECT`, and `START`.
  - Front and rear touch panel input detection.

- **Battery Health & Metrics**
  - Real-time battery life percentage, estimated cell health based on factory design capacity (2210 mAh), battery voltage, and temperature (°C / °F).
  - 60-second live voltage history graph.

- **Display & OLED Test**
  - Pure color test patterns (Red, Green, Blue, White, Black, Grayscale, and Grid).
  - Built-in OLED Exerciser to help cycle pixels and reduce retention/burn-in.

- **Motion Sensors**
  - Live 3D artificial horizon gauge using the built-in gyroscope.
  - Real-time Accelerometer ($g$) and Gyroscope (rad/s) axis data readout.

- **Audio & Microphone**
  - Separate Left (440 Hz) and Right (880 Hz) stereo speaker sound test with embedded WAV audio tones.
  - Real-time graphical VU-meter for the integrated PS Vita microphone.

- **System & Storage Info**
  - Hardware model detection (PCH-1000 OLED vs PCH-2000 Slim vs PS TV).
  - Wi-Fi IP address and hardware MAC address readout.
  - Live storage partition capacity monitoring for `ux0:`, `ur0:`, `uma0:`, and `gro0:`.

---

## Controls

- **SEL + L / R**: Switch between diagnostic tabs
- **SEL + START**: Exit VitaDoctor
- **Square / Circle** *(Audio Tab)*: Play Left / Right speaker test tone
- **Square** *(Display Tab)*: Toggle OLED Exerciser
- **D-Pad Left / Right** *(Display Tab)*: Switch display test pattern

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

- **VitaSDK Developers**: For maintaining the open-source PS Vita SDK and toolchain.
- **TheFloW**: For his legendary contributions, documentation, and tools for the PS Vita scene.
- **xerpi**: For `vita2dlib`.

---

## License

GPL-3.0 License.
