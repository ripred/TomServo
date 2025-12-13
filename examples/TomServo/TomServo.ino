/*
 * TomServo Advanced Servo Arduino Library
 *
 * TomServo saves power in multi-servo projects by suppressing servo control pulses
 * when a servo is idle. When a move completes, TomServo can detach() the signal so
 * the servo controller sees no valid frames and stops actively driving the motor.
 * Gearbox friction holds position with drastically reduced current draw.
 *
 * By multiplexing the movements of several servos so that only one is on at a time,
 * you can run many servos from a single battery for much longer.
 *
 * v 1.0.0
 * (c) May 2022 trent m. wyatt
 *
 * v 1.1.0
 * (c) Dec 2025 trent m. wyatt
 *
 */
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
        if (0 == target) {
            target = 180;
        } else {
            target = 0;
        }

        servo.write(target, MOVE_TIME_US);
    }
}

