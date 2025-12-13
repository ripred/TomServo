/**
 * TomServoPCA9685ChainedSync.ino
 *
 * Demonstrates two TomServo advantages over the stock Servo interface:
 *
 * 1) Completion callbacks:
 *    - Servo 1 moves first.
 *    - When Servo 1 reaches 180°, its callback starts Servo 2 moving to 90°.
 *
 * 2) Synchronized arrival:
 *    - After Servo 2 reaches 90°, its callback starts BOTH servos moving back to 0
 *      over the SAME duration, even though they travel different distances.
 *    - Result: both arrive at 0 at the same time for more organic motion.
 *
 * Requirements:
 * - Adafruit PWM Servo Driver Library
 * - TomServoPCA9685 support in this library
 *
 * Wiring:
 * - PCA9685 SDA/SCL -> MCU SDA/SCL
 * - PCA9685 VCC/GND -> MCU VCC/GND (logic power)
 * - Servo power -> external 5V supply recommended (common GND with MCU + PCA)
 * - Servo signal -> PCA9685 channel outputs
 *
 * Notes:
 * - PWM frequency must be set to 50 Hz for typical hobby servos.
 * - "Detach" is emulated per-channel by outputting constant LOW (no valid servo frames).
 */
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <TomServoPCA9685.h>

static uint8_t const PCA_I2C_ADDR = 0x40;

static uint8_t const SERVO1_CH = 0;
static uint8_t const SERVO2_CH = 1;

// All durations are in microseconds.
static uint32_t const SERVO1_TO_180_US = 2000000UL; // 3.0 s
static uint32_t const SERVO2_TO_90_US  = 2000000UL; // 2.0 s
static uint32_t const BOTH_TO_0_US     = 3000000UL; // 3.0 s

static uint16_t const SERVO1_TARGET_A = 180;
static uint16_t const SERVO2_TARGET_A = 90;

static uint16_t const TARGET_HOME = 0;

enum demo_phase_t : uint8_t {
    phase_servo1_moving = 1,
    phase_servo2_moving = 2,
    phase_returning     = 3,
};

struct demo_context_t {
    uint8_t phase;
};

static demo_context_t demo = { phase_servo1_moving };

static Adafruit_PWMServoDriver pca(PCA_I2C_ADDR);

static TomServoPCA9685 servo1(pca, SERVO1_CH);
static TomServoPCA9685 servo2(pca, SERVO2_CH);

static void on_servo1_done(void * ctx) {
    demo_context_t * d = (demo_context_t *)ctx;

    if (phase_servo1_moving != d->phase) {
        return;
    }

    // Servo 1 is now at 180°.
    // Start Servo 2 moving to 90°.
    d->phase = phase_servo2_moving;
    servo2.write(SERVO2_TARGET_A, SERVO2_TO_90_US);
}

static void on_servo2_done(void * ctx) {
    demo_context_t * d = (demo_context_t *)ctx;

    if (phase_servo2_moving != d->phase) {
        return;
    }

    // At this point:
    // - Servo 1 is at 180°
    // - Servo 2 is at 90°
    //
    // Now demonstrate synchronized arrival:
    // Move BOTH back to 0 over the SAME duration, despite different distances.
    d->phase = phase_returning;

    servo1.write(TARGET_HOME, BOTH_TO_0_US); // 180° -> 0°
    servo2.write(TARGET_HOME, BOTH_TO_0_US); //  90° -> 0°
}

void setup() {
    Wire.begin();

    pca.begin();
    pca.setPWMFreq(50);

    servo1.enableDetachment(true);
    servo2.enableDetachment(true);

    // Start both at 0 (detached after begin if detachment enabled).
    servo1.begin(TARGET_HOME);
    servo2.begin(TARGET_HOME);

    // Register callbacks to chain motion.
    servo1.onComplete(on_servo1_done, &demo);
    servo2.onComplete(on_servo2_done, &demo);

    // Kick off the sequence: Servo 1 moves first.
    demo.phase = phase_servo1_moving;
    servo1.write(SERVO1_TARGET_A, SERVO1_TO_180_US);
}

void loop() {
    servo1.update();
    servo2.update();

    // Once both are back at 0, restart the demo automatically.
    if (phase_returning == demo.phase && servo1.complete() && servo2.complete()) {
        demo.phase = phase_servo1_moving;
        servo1.write(SERVO1_TARGET_A, SERVO1_TO_180_US);
    }
}

