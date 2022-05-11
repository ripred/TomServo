/**
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
 * v 1.0
 * (c) May 2022 trent m. wyatt
 *
 */

#include "TomServo.h"

static unsigned const Servo1Pin = 5;
static unsigned const Servo2Pin = 6;

static uint32_t const speed1 = 2000000L;
static uint32_t const speed2 = 2000000L;

TomServo servo1(Servo1Pin);
TomServo servo2(Servo2Pin);

void setup() {
    Serial.begin(2000000);
    uint32_t timer = millis() + 250;
    while (!Serial && millis() < timer) { /* wait for valid Serial or 0.25 second timeout */ }

    Serial.flush();
    while (Serial.available()) {
        Serial.read();
    }

    servo1.begin(DefaultPos);
    servo2.begin(DefaultPos);

    Serial.print("\nTomServo Test\n");
}


void loop() {
    static signed state = 0;

    // press ENTER in serial monitor to stop
    if (Serial.available() > 0) {
        servo1.detach();
        servo2.detach();
        Serial.print("\nStopped\n");
        while (true) { }
    }

    servo1.update();
    servo2.update();

    if (servo1.complete()) state++;
    if (servo2.complete()) state++;

    state %= 4;

    switch (state) {
        default:    break;
        case 0: servo1.write(servo1.minWidth(), speed1);    break;
        case 1: servo2.write(servo2.minWidth(), speed2);    break;
        case 2: servo1.write(servo1.maxWidth(), speed1);    break;
        case 3: servo2.write(servo2.maxWidth(), speed2);    break;
    }
}
