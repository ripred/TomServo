/**
 * TomServoSweep.ino
 * 
 * Example sweep using two servos.
 * Each servo is swept back and forth using
 * a specified amount of time.
 * 
 */
#include <TomServo.h>

int const Servo1Pin = 5;
int const Servo2Pin = 6;

TomServo servo1(Servo1Pin);
TomServo servo2(Servo2Pin);

// durations to use to move to destinations (in ms):
int const duration1 = 2000; // 2 seconds
int const duration2 = 4000; // 4 seconds

int destination1 = 0;
int destination2 = 0;

void setup() {
    servo1.begin(destination1);
    servo2.begin(destination2);
}

void loop() {
    servo1.update();
    servo2.update();

    // change directions when they reach their destinations:
    if (servo1.complete()) {
        destination1 = (destination1 == 0) ? 180 : 0;
        servo1.write(destination1, duration1);
    }

    if (servo2.complete()) {
        destination2 = (destination2 == 0) ? 180 : 0;
        servo2.write(destination2, duration2);
    }
}
