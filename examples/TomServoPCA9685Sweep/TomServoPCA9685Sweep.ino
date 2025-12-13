/**
 * TomServoPCA9685Sweep.ino
 *
 * Example: single servo on PCA9685 using TomServo semantics.
 *
 * Requirements:
 * - Install "Adafruit PWM Servo Driver Library"
 *
 * Wiring:
 * - PCA9685 SDA/SCL -> MCU SDA/SCL
 * - PCA9685 VCC/GND -> MCU VCC/GND (logic power)
 * - Servo power -> external 5V supply recommended (common GND with MCU + PCA)
 * - Servo signal -> PCA9685 channel output
 *
 * Notes:
 * - PWM frequency must be set to 50 Hz for typical hobby servos.
 * - "Detach" is emulated per-channel by outputting constant LOW (no valid servo frames).
 */
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <TomServoPCA9685.h>

static uint8_t const PCA_I2C_ADDR = 0x40;
static uint8_t const SERVO_CH = 0;

static uint32_t const MOVE_TIME_US = 2000000UL; // 2 seconds

Adafruit_PWMServoDriver pca(PCA_I2C_ADDR);
TomServoPCA9685 servo(pca, SERVO_CH);

void setup() {
    Wire.begin();

    pca.begin();
    pca.setPWMFreq(50);

    servo.enableDetachment(true);
    servo.begin(DefaultPos);
}

void loop() {
    servo.update();

    static uint16_t target = 0;

    if (servo.complete()) {
        if (0 == target) {
            target = 180;
        } else {
            target = 0;
        }

        servo.write(target, MOVE_TIME_US);
    }
}

