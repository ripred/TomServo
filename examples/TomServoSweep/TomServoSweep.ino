/**
 * TomServoSweep.ino
 *
 * Example sweep using two servos.
 * Each servo is swept back and forth using a specified amount of time.
 *
 * NOTE:
 * TomServo durations are in microseconds.
 */
#include <TomServo.h>

static uint8_t const Servo1Pin = 5;
static uint8_t const Servo2Pin = 6;

TomServo servo1(Servo1Pin);
TomServo servo2(Servo2Pin);

// durations to use to move to destinations (in microseconds):
static uint32_t const duration1_us = 2000000UL; // 2 seconds
static uint32_t const duration2_us = 4000000UL; // 4 seconds

static uint16_t destination1 = 0;
static uint16_t destination2 = 0;

void setup() {
    servo1.begin(destination1);
    servo2.begin(destination2);
}

void loop() {
    servo1.update();
    servo2.update();

    if (servo1.complete()) {
        destination1 = (0 == destination1) ? 180 : 0;
        servo1.write(destination1, duration1_us);
    }

    if (servo2.complete()) {
        destination2 = (0 == destination2) ? 180 : 0;
        servo2.write(destination2, duration2_us);
    }
}

