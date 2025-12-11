/*
 * TomServo Advanced Servo Arduino Library
 *
 * The TomServo library lets you power multiple servos
 * from a single rechargeable battery.  By putting the
 * servo const pin into a high-z state when it is not 
 * being used this disabled the driving of the servo
 * and greatly reduces the current flow.
 * 
 * By multiplexing the movements of several servos
 * so that only one is on at a time you can run up
 * to 8 servos from a single battery.
 *
 * v 1.0.0
 * (c) May 2022 trent m. wyatt
 *
 * v 1.0.1
 * (c) Dec 2025 trent m. wyatt
 *
 */
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
