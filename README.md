# TomServo

**Sophisticated Servo Management for the Discriminating User.**

TomServo is a servo control library for Arduino that focuses on:

* **Power savings** for battery‑powered multi‑servo projects
* **Smooth, timed motion** with simple easing
* **Simple API** that feels like the stock `Servo` library, but smarter

Most hobby servos will happily hold position because of gearbox friction even when no valid PWM signal is present. As long as they keep seeing servo frames, the control electronics continue to drive the motor and burn current, even when “not moving”.

TomServo leans into that:

* It **attaches** only when a move is in progress.
* It **detaches** when the move is complete (if enabled), so the servo controller sees no valid frames.
* The servo coasts on gearbox friction, current draw drops dramatically, and you get a lot more runtime out of a battery.

Ideal use cases:

* Battery‑powered robots and hexapods
* Pan/tilt rigs
* Animatronics where servos hold poses for long periods
* Any project where servo idle current is a problem

## Version 1.0.5

This version refines the motion model and internal state handling without changing the public API.

### Highlights

* Constructor now correctly initializes the starting position:
  * If no starting position is passed (`pos == -1`), the servo starts at a default angle (`DefaultPos`, 90°).
  * If a starting position is provided, it is respected and used to initialize the internal position.
* `begin()`:
  * Synchronizes the internal `pos` to the starting angle you pass.
  * Attaches, writes the starting angle, waits briefly for the servo to reach it, then detaches if auto‑detach is enabled.
* `write(target, duration)`:
  * Distributes motion evenly across the requested duration.
  * Uses microsecond timing via `micros()`.
* `update()`:
  * Prevents overshoot by clamping step size so it never moves beyond the target, even if `loop()` is delayed.
* Comments and documentation now clearly state that:
  * **Positions are angles in degrees** (same units as `Servo::write()`).
  * **Durations are in microseconds**.

## Features

* **Angle‑based API**:
  * Positions are in degrees (0–180), just like the standard `Servo` library.
* **Timed motion**:
  * `write(target, duration)` moves from the current angle to `target` over a fixed duration (in microseconds).
  * Motion is advanced in small increments inside `update()`.
* **Automatic detach**:
  * Per‑instance control via `enableDetachment(true/false)`.
  * When a motion completes, the servo is optionally detached so it does not continuously draw current.
* **Explicit `update()` loop**:
  * Call `update()` frequently from `loop()` to advance any in‑progress motions.
* **State queries**:
  * `position()` returns the current servo angle in degrees.
  * `complete()` reports whether the current scheduled motion is finished.
  * `attached()` mirrors the underlying `Servo::attached()`.

## API Overview

### Construction

    TomServo(int pin,
             int min = MinWidth,
             int max = MaxWidth,
             int pos = -1);

* `pin` – Arduino pin the servo is connected to.
* `min`, `max` – pulse width limits (in microseconds) passed to `Servo::attach(pin, min, max)`.
* `pos` – starting angle in degrees:
  * If `pos == -1`, the starting angle defaults to `DefaultPos` (90°).
  * If `pos >= 0`, the starting angle is initialized to that value.

There are three related constants:

    static uint16_t const MinWidth   = 544;
    static uint16_t const MaxWidth   = 2400;
    static uint16_t const DefaultPos = 90;

### Core methods

    void begin(uint32_t pos);
    void write(uint32_t pos);
    void write(uint16_t pos, uint32_t dur);
    bool update();

    bool enableDetachment(bool allow);

    void detach();
    bool attached();

    uint32_t minWidth() const;
    uint32_t maxWidth() const;
    uint32_t position() const;  // degrees
    bool complete() const;

#### `begin(pos)`

* Attaches the servo.
* Moves immediately to `pos` (degrees).
* Waits briefly for the servo to physically reach that position.
* Detaches again if auto‑detach is enabled.
* Internal `pos` is updated to match `pos`.

Use this once in `setup()` to establish a known starting position.

#### `write(pos)`

* Immediate move to `pos` (degrees).
* Attaches, writes the new angle, and detaches if auto‑detach is enabled.

This is similar to calling `Servo::write()` but with TomServo’s attach/detach behavior.

#### `write(pos, dur)`

* Schedules a timed move from the current angle to `pos` (degrees).
* `dur` is the total move time in **microseconds**.
* Internally:
  * Computes `delta = abs(target - current)` in degrees.
  * Computes `us_per_inc = dur / delta`.
  * Marks the servo as “in motion”.
* Actual movement occurs incrementally in `update()`.

#### `update()`

Call this often from `loop()`. It:

* Checks how much time has passed since the last step using `micros()`.
* Computes how many 1‑degree steps to apply.
* Clamps the step count so it never overshoots the remaining `delta`.
* Writes the new angle via `Servo::write()`.
* When `delta` reaches 0:
  * Marks the motion as complete.
  * Detaches the servo if auto‑detach is enabled.

Return value:

* `true` – No motion is in progress and nothing needs to be done for this servo.
* `false` – Motion is in progress or work was just done this call.

In practice, you’ll usually ignore the return value and use `complete()` to decide when to schedule a new move.

#### `enableDetachment(allow)`

Enable or disable auto‑detach behavior:

    servo.enableDetachment(true);  // default; detach when done
    servo.enableDetachment(false); // keep servo attached after moves

#### `position()`

Returns the current servo angle in degrees (same units you pass to `write()` and `begin()`).

#### `complete()`

Returns `true` when there is no motion in progress and the last move has reached its target.

## Simple example: single servo sweep

Basic sweep between two angles with timed motion.

    #include <TomServo.h>

    static uint8_t const SERVO_PIN = 9;
    static uint32_t const MOVE_TIME_US = 2000000UL; // 2 seconds

    TomServo servo(SERVO_PIN);

    void setup() {
        // Start at the default angle (90°)
        servo.begin(DefaultPos);
    }

    void loop() {

        // Allow the servo to advance toward any scheduled target
        servo.update();

        static uint16_t target = 0;

        if (servo.complete()) {

            if (0 == target) {
                target = 180;
            } else {
                target = 0;
            }

            // Timed move from current angle to target over MOVE_TIME_US
            servo.write(target, MOVE_TIME_US);
        }
    }

Because TomServo automatically detaches when a move completes (by default), the servo will hold its position by friction without continuously drawing as much current.

## Multi‑servo power‑saving example

Two servos, with moves interleaved so only one is actively driven at a time. This pattern scales well to many servos on a shared battery.

    #include <TomServo.h>

    static uint8_t const SERVO1_PIN = 5;
    static uint8_t const SERVO2_PIN = 6;

    static uint32_t const MOVE_TIME1_US = 1500000UL; // 1.5 s
    static uint32_t const MOVE_TIME2_US = 2500000UL; // 2.5 s

    TomServo servo1(SERVO1_PIN);
    TomServo servo2(SERVO2_PIN);

    void setup() {
        servo1.enableDetachment(true);
        servo2.enableDetachment(true);

        // Start the servos at opposite ends of travel
        servo1.begin(0);
        servo2.begin(180);
    }

    void loop() {
        servo1.update();
        servo2.update();

        static uint16_t target1 = 180;
        static uint16_t target2 = 0;
        static uint8_t next_servo = 1;

        if (1 == next_servo && servo1.complete() && servo2.complete()) {

            target1 = (0 == target1) ? 180 : 0;
            servo1.write(target1, MOVE_TIME1_US);

            next_servo = 2;
        }

        if (2 == next_servo && servo1.complete() && servo2.complete()) {

            target2 = (0 == target2) ? 180 : 0;
            servo2.write(target2, MOVE_TIME2_US);

            next_servo = 1;
        }
    }

With this pattern:

* At most one servo is actively moving at any time.
* Both servos detach when they reach their targets.
* Total current draw stays much lower than driving both servos continuously.

## Installation

TomServo is compatible with the Arduino IDE:

1. Install via Arduino Library Manager **or**
2. Download or clone this repository into your `libraries/` folder.

Include it in your sketch with:

    #include <TomServo.h>

If you use TomServo in a project (especially a hexapod or any multi‑servo robot), feel free to share what you build.

