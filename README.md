# CrowdLens — Real-Time Crowd Monitoring System

A real-time crowd monitoring application written in C++ for the Raspberry Pi 5. It processes a
live video feed to detect crowd build-up, identify choke points, and predict stampede risk before
it escalates. Detection zones are user-defined polygons loaded from a config file, and all
thresholds are adjustable at runtime without recompiling.

Built for ENG 5220 Real-Time Embedded Programming at the University of Glasgow.

---

## How it works

Frames are captured from a camera (or a local video file for testing) and fed to an analysis
thread via a condition variable — no polling, no busy-waiting. Each frame is processed through
OpenCV background subtraction and optical flow to estimate crowd density and movement per zone.
When a threshold is crossed, alert callbacks fire immediately. A separate predictor tracks density
and flow trends over a sliding window to warn of stampede or chokepoint conditions before they
reach threshold.

The system runs three threads: capture, analysis, and display. The display is driven from the
main thread via `tick()`. See [docs/architecture.md](docs/architecture.md) for the full design.

---

## Hardware

- Raspberry Pi 5 (8 GB recommended)
- Wide-Angle 1080p UVC-Compliant USB Camera Module
- USB-C power supply (5V / 5A)
- MicroSD card (32 GB+, Class 10 or better)

<!-- TODO: Add link to docs/hardware-setup.md once hardware setup guide is written -->

---

## Software prerequisites

- CMake 3.16 or newer
- GCC 12+ or Clang 15+ (C++20 required)
- OpenCV 4.x

On Debian/Ubuntu (including Raspberry Pi OS):

```bash
sudo apt update
sudo apt install cmake build-essential libopencv-dev git
```

GTest is fetched automatically at configure time — no manual install needed.

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
the polygon corners. See [docs/architecture.md](docs/architecture.md) for the full list of
config keys.

---

## Architecture

The system is event-driven and uses a callback/subscription model throughout. The key classes are:

- `CameraFrameSource` / `VideoFileFrameSource` — frame delivery
- `CrowdAnalyser` — analysis thread, threshold checks, alert dispatch
- `StampedePredictor` — sliding-window trend prediction
- `FrameOverlay` — display window
- `ConsoleEventHandler` — console alert output

See [docs/architecture.md](docs/architecture.md) for the full class list, data-flow diagram,
threading model, SOLID rationale, and latency budget.

---

## Demo

<!-- TODO: Add demo video once hardware is available -->

---

## Social media

<!-- TODO: Add social media link once account is set up -->

---

## Licence

MIT — see [LICENSE](LICENSE).

---

## Team


University of Glasgow — ENG 5220 Real-Time Embedded Programming, 2025/26
