/**
 * TomServoPCA9685Multi.ino
 *
 * Multi-servo power-saving example using a PCA9685.
 *
 * This mirrors the README multi-servo interleaving pattern:
 * - Only one servo is actively driven (valid PWM frames) at a time.
 * - The other servo(s) are "detached" by per-channel signal suppression
 *   (emulated detach: constant LOW / no valid servo frames).
 *
 * Requirements:
 * - Adafruit PWM Servo Driver Library
 * - TomServoPCA9685 support in this repo (TomServoPCA9685.h)
 *
 * Wiring:
 * - PCA9685 SDA/SCL -> MCU SDA/SCL
 * - PCA9685 VCC/GND -> MCU VCC/GND (logic power)
 * - Servo power -> external 5V supply recommended (common GND with MCU + PCA)
 * - Servo signal -> PCA9685 channel outputs
 */
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <TomServoPCA9685.h>

static uint8_t const PCA_I2C_ADDR = 0x40;

static uint8_t const SERVO1_CH = 0;
static uint8_t const SERVO2_CH = 1;

// Move durations in microseconds:
static uint32_t const MOVE_TIME1_US = 1500000UL; // 1.5 seconds
static uint32_t const MOVE_TIME2_US = 2500000UL; // 2.5 seconds

Adafruit_PWMServoDriver pca(PCA_I2C_ADDR);

TomServoPCA9685 servo1(pca, SERVO1_CH);
TomServoPCA9685 servo2(pca, SERVO2_CH);

void setup() {

    Wire.begin();

    pca.begin();
    pca.setPWMFreq(50); // typical analog hobby servo frame rate

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

    // Only schedule a new move when both are idle, and alternate which servo moves.
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

