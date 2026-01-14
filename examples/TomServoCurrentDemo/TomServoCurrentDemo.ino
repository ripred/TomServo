/*
    TomServo + PCA9685 + CPUVolt "Current / Sag" Demo
    -------------------------------------------------
    Uses CPUVolt's readVcc() (mV) as a proxy for load/current draw while exercising
    a PCA9685-driven servo.

    CPUVolt is free functions (not a class):
        long readVcc();
        float readPercent(...);

    Recommended wiring for meaningful results:
      - Nano powered from USB hub
      - PCA9685 V+ powered from Nano 5V (same rail CPUVolt measures)
      - Common GND between Nano and PCA9685
*/

#include <Wire.h>

#include <CPUVolt.h>

#include <Adafruit_PWMServoDriver.h>
#include <TomServoPCA9685.h>

// ----------------------------
// User configuration
// ----------------------------

static uint8_t const I2C_ADDR = 0x40;

// PCA9685 PWM frequency for servos (Hz)
static uint16_t const SERVO_HZ = 50;

// Typical hobby servo pulse bounds (microseconds)
static uint16_t const SERVO_MIN_US = 500;
static uint16_t const SERVO_MAX_US = 2500;

// Measurement control
static uint32_t const MEASURE_WINDOW_MS = 8000;     // time spent in each test case
static uint16_t const SAMPLES_PER_READING = 32;     // averaged ADC-derived Vcc samples
static uint32_t const READING_PERIOD_MS = 500;      // how often to print averaged readings

// Sweep control (case 4)
static uint16_t const SWEEP_STEP_DEG = 2;
static uint32_t const SWEEP_STEP_MS = 25;
static uint8_t const SWEEP_MIN_DEG = 15;
static uint8_t const SWEEP_MAX_DEG = 165;

// Multi-servo demo
static bool const RUN_MULTI_SERVO_TEST = true;
static uint8_t const NUM_SERVOS = 5;               // <= 16
static uint32_t const MULTI_MOVE_US = 900000UL;
static uint32_t const MULTI_PAUSE_MS = 250;

// ----------------------------
// Globals
// ----------------------------

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(I2C_ADDR);

// TomServo PCA9685 wrapper for channel 0
TomServoPCA9685 ts0(pwm, 0, SERVO_MIN_US, SERVO_MAX_US, -1, SERVO_HZ);

// ----------------------------
// Helper: PCA9685 "disable channel" (constant LOW)
// Matches TomServoPCA9685 detach emulation behavior described in README.
// ----------------------------

static void pca_disable_channel_constant_low(uint8_t channel)
{
    pwm.setPWM(channel, 0, 0);
}

// ----------------------------
// Measurement helpers
// ----------------------------

static long read_vcc_avg_mv(uint16_t samples)
{
    long sum = 0;
    for (uint16_t i = 0; i < samples; i++) {
        sum += readVcc();          // CPUVolt is free function
        delay(2);
    }
    return sum / (long)samples;
}

static void print_reading(char const * label, long mv)
{
    Serial.print(label);
    Serial.print(": vcc_mv=");
    Serial.println(mv);
}

static void measure_for_window(char const * label, uint32_t window_ms, void (*loop_fn)())
{
    Serial.println();
    Serial.println("------------------------------------------------------------");
    Serial.print("CASE: ");
    Serial.println(label);
    Serial.println("------------------------------------------------------------");

    uint32_t start_ms = millis();
    uint32_t next_print_ms = start_ms;

    while ((millis() - start_ms) < window_ms) {
        if (nullptr != loop_fn) {
            loop_fn();
        }

        uint32_t now_ms = millis();
        if ((int32_t)(now_ms - next_print_ms) >= 0) {
            long mv = read_vcc_avg_mv(SAMPLES_PER_READING);
            print_reading(label, mv);
            next_print_ms = now_ms + READING_PERIOD_MS;
        }
    }
}

// ----------------------------
// Test case loop bodies
// ----------------------------

static void loop_idle()
{
    // Nothing.
}

static uint8_t hold_angle = 90;
static bool hold_inited = false;

static void loop_hold_enabled()
{
    if (!hold_inited) {
        // PCA9685 keeps outputting continuously after the registers are set.
        // TomServoPCA9685::write(pos) will enable output for that angle.
        ts0.write(hold_angle);
        hold_inited = true;
    }

    ts0.update();
}

static void loop_sweep_enabled()
{
    static uint32_t next_step_ms = 0;
    static int16_t angle = 90;
    static int8_t dir = 1;

    uint32_t now_ms = millis();
    if ((int32_t)(now_ms - next_step_ms) >= 0) {
        angle += dir * (int16_t)SWEEP_STEP_DEG;
        if (angle >= (int16_t)SWEEP_MAX_DEG) {
            angle = (int16_t)SWEEP_MAX_DEG;
            dir = -1;
        } else if (angle <= (int16_t)SWEEP_MIN_DEG) {
            angle = (int16_t)SWEEP_MIN_DEG;
            dir = 1;
        }

        ts0.write((uint16_t)angle);   // immediate write (enabled output)
        next_step_ms = now_ms + SWEEP_STEP_MS;
    }

    ts0.update();
}

static void loop_tomservo_sweep()
{
    static uint32_t next_schedule_ms = 0;
    static uint8_t target = SWEEP_MAX_DEG;

    uint32_t now_ms = millis();

    ts0.update();

    if ((int32_t)(now_ms - next_schedule_ms) >= 0) {
        // Timed move; TomServoPCA9685 will "detach" (constant LOW) when complete if enabled.
        ts0.write(target, 600000UL);

        if (target == SWEEP_MAX_DEG) {
            target = SWEEP_MIN_DEG;
        } else {
            target = SWEEP_MAX_DEG;
        }

        next_schedule_ms = now_ms + 700;
    }
}

// ----------------------------
// Multi-servo interlaced demonstration
// ----------------------------

static void run_multi_servo_demo()
{
    Serial.println();
    Serial.println("============================================================");
    Serial.println("MULTI-SERVO DEMO (TomServo interlaced timing)");
    Serial.println("============================================================");

    // Create channels 0..4 (or fewer) with consistent min/max/hz.
    static TomServoPCA9685 s0(pwm, 0, SERVO_MIN_US, SERVO_MAX_US, -1, SERVO_HZ);
    static TomServoPCA9685 s1(pwm, 1, SERVO_MIN_US, SERVO_MAX_US, -1, SERVO_HZ);
    static TomServoPCA9685 s2(pwm, 2, SERVO_MIN_US, SERVO_MAX_US, -1, SERVO_HZ);
    static TomServoPCA9685 s3(pwm, 3, SERVO_MIN_US, SERVO_MAX_US, -1, SERVO_HZ);
    static TomServoPCA9685 s4(pwm, 4, SERVO_MIN_US, SERVO_MAX_US, -1, SERVO_HZ);

    TomServoPCA9685 * multi[5] = { &s0, &s1, &s2, &s3, &s4 };

    uint8_t const n = (NUM_SERVOS > 5) ? 5 : NUM_SERVOS;

    for (uint8_t i = 0; i < n; i++) {
        multi[i]->enableDetachment(true);
        multi[i]->begin(90);           // begin(pos) takes one arg
    }

    uint8_t phase = 0;
    uint32_t start_ms = millis();
    uint32_t next_print_ms = start_ms;

    uint32_t const demo_ms = 15000;

    while ((millis() - start_ms) < demo_ms) {
        for (uint8_t i = 0; i < n; i++) {
            multi[i]->update();
        }

        static uint32_t next_schedule_ms = 0;
        uint32_t now_ms = millis();

        if ((int32_t)(now_ms - next_schedule_ms) >= 0) {
            uint8_t idx = phase % n;
            uint8_t target = ((phase & 1) == 0) ? SWEEP_MAX_DEG : SWEEP_MIN_DEG;

            multi[idx]->write(target, MULTI_MOVE_US);

            phase++;
            next_schedule_ms = now_ms + MULTI_PAUSE_MS;
        }

        if ((int32_t)(now_ms - next_print_ms) >= 0) {
            long mv = read_vcc_avg_mv(SAMPLES_PER_READING);
            print_reading("MULTI_TOMSERVO_INTERLACED", mv);
            next_print_ms = now_ms + READING_PERIOD_MS;
        }
    }

    Serial.println("MULTI-SERVO DEMO DONE.");
}

// ----------------------------
// Setup / Loop
// ----------------------------

void setup()
{
    Serial.begin(115200);
    while (!Serial) { }

    delay(300);

    Serial.println();
    Serial.println("TomServo + PCA9685 + CPUVolt Power Savings Demo");
    Serial.println("------------------------------------------------------------");
    Serial.println("CPUVolt measures Vcc (Nano 5V rail). Power PCA9685 V+ from 5V pin.");
    Serial.println();

    Wire.begin();

    pwm.begin();
    pwm.setPWMFreq(SERVO_HZ);

    // Initialize TomServoPCA9685 wrapper
    ts0.enableDetachment(true);
    ts0.begin(90);

    // Start with channel disabled (constant low)
    pca_disable_channel_constant_low(0);

    Serial.println("Starting cases in 2 seconds...");
    delay(2000);
}

void loop()
{
    // CASE 1: Baseline (PCA9685 attached; ideally NO servo plugged in)
    measure_for_window("1_BASELINE_NO_SERVO", MEASURE_WINDOW_MS, loop_idle);

    // CASE 2: Servo attached, PWM disabled (constant LOW)
    pca_disable_channel_constant_low(0);
    measure_for_window("2_SERVO_ATTACHED_PWM_DISABLED", MEASURE_WINDOW_MS, loop_idle);

    // CASE 3: Servo attached, PWM enabled, holding fixed position
    hold_inited = false;
    measure_for_window("3_SERVO_HOLD_PWM_ENABLED", MEASURE_WINDOW_MS, loop_hold_enabled);

    // CASE 4: Servo attached, sweeping with PWM enabled
    measure_for_window("4_SERVO_SWEEP_PWM_ENABLED", MEASURE_WINDOW_MS, loop_sweep_enabled);

    // CASE 5: TomServo timed sweep with detach emulation (the key comparison)
    // This is the “TomServo” version of motion: drive only during the move,
    // then output constant LOW when idle.
    pca_disable_channel_constant_low(0);
    measure_for_window("5_TOMSERVO_TIMED_SWEEP_WITH_DETACH", MEASURE_WINDOW_MS, loop_tomservo_sweep);

    if (RUN_MULTI_SERVO_TEST) {
        run_multi_servo_demo();
    }

    Serial.println();
    Serial.println("All tests complete. Restarting in 10 seconds...");
    delay(10000);
}

