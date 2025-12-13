# TomServo

**Sophisticated Servo Management for the Discriminating User.**

TomServo is an Arduino servo control library focused on:

* **Big power savings** for battery-powered multi-servo projects
* **Smooth timed motion** (duration in microseconds) using an `update()` loop
* **Simple, Servo-like API** — but smarter

Most hobby servos will happily hold position because of gearbox friction even when no valid PWM signal is present. As long as they keep seeing servo frames, the control electronics continue to drive the motor and burn current, even when “not moving”.

TomServo leans into that:

* It **attaches** only when a move is in progress.
* It **detaches** when a move is complete (if enabled), so the servo sees no valid frames.
* The servo holds by friction, current draw drops dramatically, and battery runtime improves.

## Quick start

### Install

TomServo is compatible with the Arduino IDE:

1. Install via Arduino Library Manager **or**
2. Download or clone this repository into your `libraries/` folder.

Include it in your sketch with:

    #include <TomServo.h>

If you want PCA9685 support, also install:

* **Adafruit PWM Servo Driver Library** (Library Manager)

## Examples (start here)

### 1) Single servo timed sweep (GPIO pin)

    #include <TomServo.h>

    static uint8_t const SERVO_PIN = 9;
    static uint32_t const MOVE_TIME_US = 2000000UL; // 2 seconds

    TomServo servo(SERVO_PIN);

    void setup() {
        servo.begin(DefaultPos);
    }

    void loop() {
        servo.update();

        static uint16_t target = 0;

        if (servo.complete()) {
            target = (0 == target) ? 180 : 0;
            servo.write(target, MOVE_TIME_US);
        }
    }

Also available as an Arduino example sketch:

* `File → Examples → TomServo → TomServo`

### 2) Multi-servo power-saving pattern (interleaved motion)

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

Also available as an Arduino example sketch:

* `File → Examples → TomServo → TomServoSweep`

## New in 1.1.0

Version **1.1.0** adds two major capabilities:

### PCA9685 support (TomServoPCA9685)

TomServo can optionally drive servos through a PCA9685 (16-channel I2C PWM expander) using the **same TomServo semantics** (degrees positions, microsecond durations, `update()` timing engine).

Important PCA9685 constraint: there is **no per-channel tri-state** and only one global OE (not suitable for per-servo detach). TomServoPCA9685 preserves the power-saving model by emulating per-servo detach:

* **Attach (emulated):** write a valid servo pulse for that channel (angle → µs → ticks)
* **Detach (emulated):** output an invalid servo waveform per channel (constant LOW)

Example sketches:

* `File → Examples → TomServo → TomServoPCA9685Sweep`
* `File → Examples → TomServo → TomServoPCA9685Multi`
* `File → Examples → TomServo → TomServoPCA9685ChainedSync`

### Completion callbacks (motion chaining)

You can register a callback that fires **once** when a timed move reaches its target:

* Only for `write(target, duration)` (timed moves)
* Not for `begin()` or `write(target)` immediate moves
* Intended for chaining a dynamic number of motions

The callback demo sketch:

* `File → Examples → TomServo → TomServoPCA9685ChainedSync`

### Synchronized arrival (organic motion)

A very practical animation trick: command multiple servos to move over the **same total duration**, even if they travel different distances, so they **arrive together**.

This creates motion that looks more fluid/intentional (especially for animatronics), and it’s naturally supported by TomServo’s timed motion model.

The `TomServoPCA9685ChainedSync` example demonstrates:

1) Servo 1 moves and triggers Servo 2 via callback  
2) Both servos then return to 0 over the same duration and arrive together

## Core concepts (what makes TomServo different)

### Degrees-based positions

TomServo positions are angles in degrees (0–180 typical), just like the stock `Servo` library.

### Microsecond durations

Timed moves use **microseconds** for duration:

* `write(target, duration_us)`

### Auto-detach power saving

When enabled (default), TomServo detaches after motion completes:

* GPIO/Servo backend: `Servo::detach()` stops the pulses entirely
* PCA9685 backend: per-channel output is set to an invalid servo waveform (constant LOW)

In both cases, the servo is no longer being actively driven while idle.

## API overview (reference)

### Construction

    TomServo(int pin,
             int min = MinWidth,
             int max = MaxWidth,
             int pos = -1);

* `pin` – Arduino pin the servo is connected to.
* `min`, `max` – pulse width limits (microseconds) passed to `Servo::attach(pin, min, max)`.
* `pos` – starting angle in degrees:
  * If `pos == -1`, the starting angle defaults to `DefaultPos` (90°).
  * If `pos >= 0`, the starting angle is initialized to that value.

Constants:

    static uint16_t const MinWidth   = 544;
    static uint16_t const MaxWidth   = 2400;
    static uint16_t const DefaultPos = 90;

### Core methods

    void begin(uint32_t pos);
    void write(uint32_t pos);
    void write(uint16_t pos, uint32_t dur_us);
    bool update();

    bool enableDetachment(bool allow);

    void onComplete(tomservo_on_complete_cb_t cb, void * ctx);

    void detach();
    bool attached();

    uint32_t minWidth() const;
    uint32_t maxWidth() const;
    uint32_t position() const;  // degrees
    bool complete() const;

#### `begin(pos)`

* Attaches the servo
* Moves immediately to `pos` (degrees)
* Waits briefly for the servo to physically reach that position
* Detaches again if auto-detach is enabled
* Internal `pos` is updated to match `pos`

Use once in `setup()` to establish a known starting position.

#### `write(pos)`

Immediate move to `pos` (degrees). Attaches, writes, and detaches if enabled.

#### `write(pos, dur_us)`

Schedules a timed move from the current angle to `pos` over **dur_us microseconds**.

Internally:

* `delta = abs(target - current)` in degrees
* `us_per_inc = dur_us / delta`

Actual movement occurs incrementally in `update()`.

#### `update()`

Call frequently from `loop()`:

* uses `micros()` to compute how many 1-degree steps to apply
* clamps step count so it never overshoots
* writes the new angle
* when motion completes:
  * marks complete
  * detaches if enabled
  * fires the completion callback (if registered)

Return value:

* `true` – no motion is in progress and nothing needs doing
* `false` – motion in progress or work performed this call

## Version history

### Version 1.1.0

* Optional PCA9685 support via `TomServoPCA9685` (Adafruit driver)
* Optional completion callback for chaining timed motion
* PCA9685 examples: sweep, multi-servo interleaving, chained+sync demo

### Version 1.0.5

Refined motion model and internal state handling without changing the public API.

* Constructor initializes starting position correctly (default 90° if `pos == -1`)
* `begin()` synchronizes internal `pos`, attaches, writes, waits, detaches if enabled
* `write(target, duration)` uses microsecond timing via `micros()`
* `update()` clamps steps to prevent overshoot
* Documentation clarifies:
  * positions are degrees
  * durations are microseconds

