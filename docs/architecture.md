# CrowdLens — Architecture Reference

## 1. System Overview

CrowdLens is a real-time crowd monitoring system running on a **Raspberry Pi 5** with a camera
module. It detects crowd density, identifies choke points, and analyses flow direction within
user-defined zones. Threshold-crossing events trigger callbacks without polling. A predictive layer
(`StampedePredictor`) tracks density and flow trends over a sliding window and fires early warnings
before conditions escalate to a stampede or chokepoint.

**Target latency:** frame processing must complete within one frame period (≤ 33 ms at 30 fps)
so that crowd state assessments are never stale.

---

## 2. Module Responsibilities

| Module          | Responsibility                                                              |
|-----------------|-----------------------------------------------------------------------------|
| CameraCapture   | Acquires frames from the camera or a video file; delivers them via callback. |
| AIDetection     | Analyses frames per zone; computes density, count, and flow metrics; fires alert callbacks when thresholds are exceeded; runs predictive risk assessment. |
| Notification    | Receives alert events via the `AlertRunnable` interface and handles them (console output, hardware, etc.). |
| Config          | Reads `config.yaml` at startup; provides all runtime-tunable parameters. |
| Display         | Owns the OpenCV window; receives frames via `DisplayCallback` and alerts via `AlertRunnable`; driven by `tick()` from the main thread. |
| HardwareOutput  | (Future) Controls physical output devices (speaker, LED).                  |
| IRSensor        | (Future) Integrates IR trip-wire counts.                                   |
| demo            | Entry point; wires all modules together and starts the pipeline.           |

---

## 3. Complete Class List

| Class / Struct           | Module          | Role                                                       |
|--------------------------|-----------------|------------------------------------------------------------|
| `Runnable`               | Common          | Base interface: `run(TrafficState)`, `stop(TrafficState)`. |
| `TrafficEventHandler`    | Common          | Abstract broadcaster; manages subscriber lists and the analysis worker thread. |
| `AlertRunnable`          | Common          | Extends `Runnable`; adds `onAlert(AlertEvent)` for alert subscribers. |
| `CrowdMetrics`           | Common          | Immutable per-zone snapshot: density, count, flow angle, flow magnitude. |
| `AlertEvent`             | Common          | Value-type threshold-crossing event with type, severity, and metrics. |
| `IFrameSource`           | CameraCapture   | Pure abstract interface for frame-producing sources.       |
| `CameraFrameSource`      | CameraCapture   | Wraps `cv::VideoCapture`; dedicated capture thread.        |
| `VideoFileFrameSource`   | CameraCapture   | Wraps `cv::VideoCapture` for local video files; same interface as `CameraFrameSource`. |
| `Zone`                   | AIDetection     | Named polygon region; `contains()` test.                  |
| `ZoneManager`            | AIDetection     | Thread-safe registry of `Zone` objects.                   |
| `IFrameProcessor`        | AIDetection     | Pure abstract interface for frame analysis strategies.    |
| `OpenCVFrameProcessor`   | AIDetection     | Concrete: background subtraction + optical flow.          |
| `CrowdAnalyser`          | AIDetection     | Orchestrator; extends `TrafficEventHandler`; bounded queue (cap 1); analysis thread. |
| `StampedePredictor`      | AIDetection     | Tracks per-zone density/flow history; predicts stampede or chokepoint risk within a configurable horizon. Owned by `CrowdAnalyser`. |
| `AppConfig`              | Config          | Plain struct holding all runtime-tunable parameters (thresholds, optical-flow params, source, zones). |
| `ConfigLoader`           | Config          | Reads `config.yaml`; missing keys fall back to `AppConfig` defaults. |
| `FrameOverlay`           | Display         | OpenCV window; extends `AlertRunnable` for the alert banner; driven by `tick()` on the main thread. |
| `ConsoleEventHandler`    | Notification    | Extends `AlertRunnable`; prints alert details to stdout.  |

---

## 4. Data-Flow Diagram

```
── Startup ──────────────────────────────────────────────────────────────────
ConfigLoader::load("config.yaml")
    cfg.cameraIndex == -1 ?
        yes → VideoFileFrameSource(cfg.videoFilePath)
        no  → CameraFrameSource(cfg.cameraIndex)

── Capture → Analysis ───────────────────────────────────────────────────────
[OS / camera driver]
    │ frame ready — VideoCapture::read() unblocks (OS-woken, no busy-wait)
    ▼
CameraFrameSource / VideoFileFrameSource  [capture thread]
    │ calls FrameCallback → analyser.onFrameArrived()
    ▼
CrowdAnalyser::onFrameArrived()
    │ acquires frameMutex_, replaces pendingFrame_ (drop-oldest),
    │ calls condition_variable::notify_one()
    ▼
CrowdAnalyser  [analysis thread — sleeping on condition_variable]
    │ wakes, takes pendingFrame_, releases lock
    │ calls IFrameProcessor::processFrame(frame, zones, ts)
    │     → vector<CrowdMetrics>
    │ StampedePredictor::update(metrics)  — updates per-zone history window
    │ StampedePredictor::predict(zoneName) — returns Prediction (stampede/chokepoint risk)
    │ checkThresholds() — compares metrics against densityThreshold_ / chokepointThreshold_
    │ if exceeded → fireAlert(AlertEvent) → alertCallback(ev)
    │ calls DisplayCallback → overlay.pushFrame(frame, metrics, zones)
    ▼
alertCallback(AlertEvent)
    │ iterates alertRunnables_, calls onAlert() on each
    ▼
AlertRunnable::onAlert()   [one call per registered subscriber]
    ConsoleEventHandler → stdout
    FrameOverlay::onAlert() → queues alert banner text
    (future) HardwareAlertSink → GPIO

── Main thread loop ─────────────────────────────────────────────────────────
while (overlay.tick()) {}
    tick() calls cv::waitKey(1) — 1 ms yield, not a busy-wait
    returns false when the user presses 'q' or closes the window
```

---

## 5. Threading Model

| Thread              | Owner                | Wakes on                              | Sleeps on                          |
|---------------------|----------------------|---------------------------------------|------------------------------------|
| Main thread         | `main()`             | `FrameOverlay::tick()` returns        | `cv::waitKey(1)` inside `tick()`   |
| Capture thread      | `CameraFrameSource`  | OS frame-ready event via `read()`     | `cv::VideoCapture::read()` blocks  |
| Analysis thread     | `CrowdAnalyser`      | `condition_variable::notify_one()`    | `condition_variable::wait()`       |

**Inter-thread communication:** the only shared state between capture and analysis threads is
`pendingFrame_` (an `std::optional`), protected by `frameMutex_`. The capture thread writes;
the analysis thread reads. A `std::condition_variable` is the sole waking mechanism — no polling.

---

## 6. Frame Drop Policy

**Policy: drop-oldest (queue capacity = 1)**

`CrowdAnalyser::onFrameArrived()` unconditionally replaces `pendingFrame_` with the new frame
before notifying the analysis thread. If the analysis thread is still processing the previous
frame when a new one arrives, the previous *pending* (not yet analysed) frame is discarded.

**Rationale:** crowd state data is most useful when fresh. Accumulating a backlog of stale frames
would introduce latency that makes the density and flow readings meaningless for real-time
intervention. Dropping the oldest pending frame keeps the system self-healing under load spikes
without requiring a complex queue-management strategy.

**Frame drop is not logged by default** but a counter can be added inside `onFrameArrived()` for
diagnostic telemetry.

---

## 7. Real-Time Justification

The system targets a **soft real-time** guarantee: each frame must be analysed within one frame
period so that crowd state is never more than one frame stale.

- At 30 fps the frame period is **33 ms**.
- `CameraFrameSource` delivers frames from a dedicated OS-scheduled thread; `VideoCapture::read()`
  blocks on a kernel-level V4L2 event, so no CPU cycles are wasted between frames.
- `CrowdAnalyser` wakes immediately on `condition_variable::notify_one()` — no timer polling.
- If OpenCV processing takes longer than 33 ms on a given frame, the next arriving frame replaces
  the pending one (drop-oldest). The analysis thread processes the freshest available frame as
  soon as it finishes, bounding worst-case staleness to one additional frame period.
- All threshold checks and callback invocations happen synchronously inside the analysis thread,
  so alert latency from threshold crossing to `onAlert()` is bounded by the time to complete
  `checkThresholds()` — a constant-time operation proportional to the number of zones.

### Latency budget (estimated)

These are design-time estimates based on code structure and typical OpenCV operation costs.
Actual measurements will be recorded once the system is running on Pi 5 hardware.

<!-- TODO: Replace estimates with measured values once running on Pi 5 -->

| Stage | Estimated budget | Basis |
|---|---|---|
| Capture callback overhead | < 1 ms | Mutex acquire + optional replace |
| Analysis (OpenCV) | ≤ 25 ms | MOG2 background subtraction + Farneback optical flow |
| Threshold + predictor checks | < 1 ms | O(n zones), n ≤ ~10 |
| Alert dispatch (fan-out) | < 1 ms | Iterate registered `AlertRunnable` subscribers |
| Display push | < 1 ms | Mutex acquire + optional replace |
| **Total target** | **≤ 28 ms** | Within 33 ms frame period at 30 fps |

---

## 8. Predictive Alerts — StampedePredictor

`StampedePredictor` is owned by `CrowdAnalyser` and called on every analysis cycle. It maintains
a per-zone sliding window of samples (density + flow magnitude + timestamp) and fits a linear
trend to detect whether conditions are worsening fast enough to breach a threshold within a
configurable look-ahead horizon.

**How it works:**

After each frame is processed, `CrowdAnalyser` calls `predictor_.update(metrics)` for each zone.
`StampedePredictor` appends the sample to the zone's history deque (capped at `windowSize_`
samples). It then computes:

- **density slope** — rate of change of density over the history window (density units per second)
- **flow slope** — rate of change of flow magnitude over the window

`predict(zoneName)` returns a `Prediction` struct:
- `stampedeRisk = true` if density is rising fast enough to reach `stampedeDensityThreshold_`
  within `horizon_` seconds
- `chokepointRisk = true` if density is rising and flow is falling such that both converge on
  chokepoint conditions within `horizon_` seconds

When `stampedeRisk` is true, `CrowdAnalyser` fires a `STAMPEDE_RISK` alert. When `chokepointRisk`
is true, it fires `CHOKEPOINT_PREDICTED`. Both use `AlertSeverity::CRITICAL`.

**Relevant config keys:**

```yaml
prediction:
  stampede_density: 0.9    # density threshold for stampede
  horizon: 10.0            # look-ahead window in seconds
  window_size: 10          # number of samples to keep per zone
  min_trend_slope: 0.005   # minimum density/sec to count as a real trend
```

---

## 9. SOLID Rationale

This section explains the design decisions in terms of SOLID principles and why they made sense
for this specific problem. The goal was to keep the codebase easy to extend without having to
touch working code, which matters here because the hardware modules (IR sensor, GPIO output) are
being added incrementally and the notification targets will expand over time.

### IFrameSource / IFrameProcessor — Open/Closed + Liskov Substitution

We separated the "where frames come from" concern (`IFrameSource`) from the "what to do with a
frame" concern (`IFrameProcessor`) because we knew from the start we'd need both a live camera
and a video file source for testing. Adding `VideoFileFrameSource` required zero changes to
`CrowdAnalyser` — it just sees an `IFrameSource`. Same goes for `IFrameProcessor`: if we want
to swap in a different detection algorithm, `CrowdAnalyser` doesn't need to change. Liskov is
satisfied because `CrowdAnalyser` only calls `setFrameCallback()`, `start()`, and `stop()` — any
concrete source can substitute for any other without breaking the pipeline.

### CrowdAnalyser — Single Responsibility

`CrowdAnalyser` runs the analysis loop, checks thresholds, and fires events. It doesn't know
how to format a console message, render a display window, or write to a GPIO pin. Those
responsibilities belong to their own classes. The practical benefit: when we redesigned the
notification system (removing `AlertDispatcher` in favour of direct `AlertRunnable` registration),
`CrowdAnalyser`'s logic didn't change — only how it was wired up in `Main.cpp`.

### Runnable / AlertRunnable — Interface Segregation

Early designs had a single callback interface with methods for both traffic-state changes and
alert events. That meant every subscriber had to implement methods it didn't care about. We split
this: `Runnable` only has `run(TrafficState)` and `stop(TrafficState)`, for components that care
about system state transitions. `AlertRunnable` extends `Runnable` and adds `onAlert()`, for
components that also need to react to threshold events. `ConsoleEventHandler` and `FrameOverlay`
implement `AlertRunnable`. A future component that only needs to start/stop on traffic state
changes would only implement `Runnable`. Nothing is forced to implement methods it doesn't use.

### ZoneManager — Single Responsibility

`ZoneManager` is just a thread-safe container for `Zone` objects. It doesn't know about thresholds,
processing algorithms, or alerts. The separation is useful because zones can be added or removed
at runtime (via `addZone()` / `removeZone()`) without touching the analysis logic. The mutex
lives in `ZoneManager`, so the analysis thread and any future UI thread can both access zones
safely without `CrowdAnalyser` having to manage synchronisation itself.

### StampedePredictor — Single Responsibility

Predictive logic and reactive threshold logic are separate for a concrete reason: they change at
different rates. If we tune the prediction algorithm (different regression method, different
horizon calculation), only `StampedePredictor` changes. If we add a new reactive threshold type,
only `CrowdAnalyser::checkThresholds()` changes. Mixing them would mean any tweak to prediction
would risk breaking the reactive checks and vice versa.

### ConsoleEventHandler / AlertRunnable — Open/Closed

The notification side of the system is closed to modification but open to extension. Adding a
new alert target (hardware buzzer, network push, file logger) means creating a new class that
extends `AlertRunnable` and registering it with `CrowdAnalyser::registerAlertRunnable()`. Nothing
in `CrowdAnalyser` or `ConsoleEventHandler` needs to change. This is directly relevant to the
project because hardware notification (GPIO) is being added separately by another team member.

---

## 10. How to Add a New Zone at Startup

Zones are defined in `config.yaml` and loaded by `ConfigLoader` before the pipeline starts:

```yaml
zones:
  names: Entrance, ExitGate

  zone.Entrance:
    type: ENTRANCE
    points: 0,0 200,0 200,400 0,400

  zone.ExitGate:
    type: EXIT
    points: 500,100 640,100 640,400 500,400
```

`ConfigLoader::load()` parses each `zone.<name>` block, and `Main.cpp` converts those into `Zone`
objects and registers them with `ZoneManager`.

Zones can also be added or removed at runtime via any thread by calling `ZoneManager::addZone()`
or `ZoneManager::removeZone()` — both are mutex-protected.

---

## 11. How to Add an Alert Subscriber

1. Create a class that extends `cl::AlertRunnable` and implements `onAlert()`.
2. In `Main.cpp`, construct an instance and register it with the analyser:

```cpp
MyCustomSink customSink;
analyser.registerAlertRunnable(customSink);
```

The subscriber's lifetime must exceed `CrowdAnalyser`'s. `stop()` is broadcast to all registered
`AlertRunnable` instances when `CrowdAnalyser::stop()` is called.

---

## 12. Build & Run Instructions

### Prerequisites

- CMake ≥ 3.16
- C++20 compiler (GCC 12+ or Clang 15+)
- OpenCV 4.x (`libopencv-dev` on Debian/Ubuntu, `opencv` on Homebrew)
- Internet access for first build (GTest fetched via FetchContent)

### Build

```bash
cmake -B build -S .
cmake --build build
```

### Run tests

```bash
ctest --test-dir build --output-on-failure
```

### Run the demo

```bash
./build/src/demo/CrowdLens
# Press 'q' in the window to stop
```

### CI

GitHub Actions (`.github/workflows/workflow.yml`) runs configure → build → test on every push
to `main` / `master` / `develop*` and on pull requests targeting those branches.
