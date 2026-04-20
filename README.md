# CrowdLens - Real-Time Crowd Monitoring System

A real-time crowd monitoring application written in C++ for the Raspberry Pi 5. It processes a
live video feed to detect crowd build-up, identify choke points, and predict stampede risk before
it escalates. Detection zones are user-defined polygons loaded from a config file, and all
thresholds are adjustable at runtime without recompiling.

Built for ENG 5220 Real-Time Embedded Programming at the University of Glasgow.

---

## How it works

Frames are captured from a camera (or a local video file for testing) and fed to an analysis
thread via a condition variable (no polling, no busy-waiting). Each frame is processed through
OpenCV background subtraction and optical flow to estimate crowd density and movement per zone.
When a threshold is crossed, alert callbacks fire immediately. A separate predictor tracks density
and flow trends over a sliding window to warn of stampede or chokepoint conditions before they
reach threshold.

The system runs three threads: capture, analysis, and display.

---

## Hardware

- Raspberry Pi 5 (8 GB recommended)
- Wide-Angle 1080p UVC-Compliant USB Camera Module
- USB-C power supply (5V / 5A)
- MicroSD card (32 GB+, Class 10 or better)
- 3 LEDs (green, red, yellow)
- Passive Buzzer
- Mini Speaker

See [docs/hardware-setup.md](docs/hardware-setup.md) for the full pin assignment table and wiring setup diagram for each hardware module.

---

## Software prerequisites

- CMake 3.16 or newer
- GCC 12+ or Clang 15+ (C++20 required)
- OpenCV 4.x
- Curl 7.78.0+
- Camera module driver (libcamera)

On Debian/Ubuntu (including Raspberry Pi OS):

```bash
sudo apt update
sudo apt install cmake build-essential libopencv-dev libcurl4-openssl-dev git libcamera-dev 
```
GTest is fetched automatically at configure time, no manual install needed.

---

## Build

```bash
git clone <repo-url>
cd CrowdLens
cmake -B build -S .
cmake --build build
```

---

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

---

## Run the demo

```bash
./build/src/demo/CrowdLens
```

Press `q` in the display window to stop.

By default the demo runs on the video file set in `src/config.yaml`. To switch to a live camera,
set `Camera:index` to `0` (or the device index shown by `v4l2-ctl --list-devices`).

---

## Configuration

All runtime parameters live in `src/config.yaml`. The main ones to know:

```yaml
Camera:
  index: -1             # -1 = use video file; 0+ = camera device index
  video_file: videos/video2.mp4

thresholds:
  density: 0.25         # triggers CONGESTION alert
  chokepoint: 0.85      # triggers CHOKEPOINT alert (combined with low flow)
  flow_magnitude: 2.0   # below this is considered low flow

zones:
  names: entrance_1, exit_1, main_hall
  zone.entrance_1:
    type: ENTRANCE
    points: 0,0 200,0 200,432 0,432
```

Zone types are `ENTRANCE`, `EXIT`, `GENERAL`, or `CHOKEPOINT`. Points are pixel coordinates of
the polygon corners.

---

## Architecture

The pipeline runs three threads: capture, analysis, and display. The capture thread blocks on
`VideoCapture::read()` and fires a callback when a frame is ready. The analysis thread sleeps on
a condition variable and wakes up when a frame arrives. If a frame arrives while the analyser is
busy, the pending one gets replaced (we only keep the latest). Alerts fire as callbacks to
registered subscribers.

Target: frame analysis must complete within 33 ms (one frame period at 30 fps). OpenCV MOG2 +
Farneback is the bottleneck, estimated at around 25 ms on Pi 5 hardware.

---

## Demo

The demo runs on any machine using a local video file. On Raspberry Pi 5 with the camera module
attached, set `Camera:index` to `0` in `config.yaml` to switch to live input.

While running, the display window shows:
- Per-zone density percentage and person count estimate
- Optical flow arrows indicating crowd movement direction and speed
- A red alert banner at the top of the frame when a threshold-crossing event fires, showing the zone name, alert type (`CONGESTION` / `CHOKEPOINT` / `STAMPEDE_RISK`), and severity

Alert text is also printed to stdout by `ConsoleEventHandler`.

<!-- TODO: Add demo video link once recorded on Pi 5 hardware -->

---

## Social media

https://www.instagram.com/crowdlens17

---

## Licence

MIT, see [LICENSE](LICENSE).

---

## Team


University of Glasgow — ENG 5220 Real-Time Embedded Programming, 2025/26

### Members:
- [Candolp Dompreh](https://github.com/candolp)
- [Bassam ZaidAlKilani](https://github.com/3097968z)
- [Amantle Bogacu](https://github.com/4TSHADI)
- [Michelle Ambunya](https://github.com/WereAM)
- [Abhishek kumar gautam](https://github.com/abhikk8171)

### Project Management:
[Project Board](https://github.com/users/candolp/projects/5)