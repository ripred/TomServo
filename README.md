<!-- [![Arduino CI](https://github.com/ripred/TomServo/workflows/Arduino%20CI/badge.svg)](https://github.com/marketplace/actions/arduino_ci) -->
[![Arduino-lint](https://github.com/ripred/TomServo/actions/workflows/arduino-lint.yml/badge.svg)](https://github.com/ripred/TomServo/actions/workflows/arduino-lint.yml)
![code size](https://img.shields.io/github/languages/code-size/ripred/TomServo)
[![GitHub release](https://img.shields.io/github/release/ripred/TomServo.svg?maxAge=3600)](https://github.com/ripred/TomServo/releases)
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](https://github.com/ripred/TomServo/blob/master/LICENSE)

# Thomas Servo Eskwire 😉

![TomServo32x32.png](TomServo32x32.png)

### Sophisticated Servo Management for the Discriminating User.

The TomServo library lets you power multiple servos
from a single rechargeable battery. It works by putting the
servo control pin into a high-z state when it is not 
moving so you disable the driving of the servo
and greatly reduce the current used by all the servos in 
your project.

By multiplexing the movements of several servos
so that only one is on at a time you can run up
to 8 servos from a single battery. Or many more
using only a few batteries!

In addition to being useful for power consumption, the library 
allows you to greatly reduce servo jitter in low torque servo 
applications. By definition, if the servo(s) don't have a lock on
a valid PWM signal then they also aren't constantly trying to adjust
their position because they think the servo is ever in the *"wrong"*
position.

**Example Use:**
```cpp
/* 
 * TomServoSweep.ino
 * 
 * Example sweep using two servos.
 * Each servo is swept back and forth over the specified amount of time.
 */
#include <TomServo.h>

int const Servo1Pin = 5;
int const Servo2Pin = 6;

TomServo servo1(Servo1Pin);
TomServo servo2(Servo2Pin);

// The time to take to move to the destinations (in ms):
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
        if (destination1 == 0) {
            destination1 = 180;
        } else {
            destination1 = 0;
        }
        servo1.write(destination1, duration1);
    }

    if (servo2.complete()) {
        if (destination2 == 0) {
            destination2 = 180;
        } else {
            destination2 = 0;
        }
        servo2.write(destination2, duration2);
    }
}
```

# Theory of Operation

A servo motor only drives the higher voltage motor drive circuitry *after it has latched onto a valid PWM position*. This is required so it can compare the current position against the new position (received via the width of a PWM pulse) and make any necessary adjustments to the position of the main drive shaft by powering the motor and moving it forwards or backwards.

Many people don't realize that the motor drive circuitry is **always** engaged and consuming current **even when the servo has reached its target position! As long as the servo is receiving a PWM pulse, it is comparing that position against the current drive shaft position and making tiny adjustments to the motor's position. This consumes a huge amount of power in projects that use servos.

If the servo is not receiving a valid PWM signal, then it does engage the motor drive circuitry. It's that simple. The driving of the motor is responsible for about ~3/4 of the total power consumption of servos so by stopping it we greatly reduce the idle power requirements of all of the servos in our project combined. 

When the `attach(pin)` method is called on a `Servo` object, then the internal `Timer1` registers are configured along with an interrupt to start generating a PWM signal on the specified pin. There is a companion method to `attach(...)` called `detach()`. When `detach()` is called on a `Servo` object, then the PWM generation is stopped. 

In the future if the position of the servo needs to be changed by calling `write(...)`, then the `attach(...)` method must also be called again to restart the PWM generation. Once a new position has been written to the servo, it must be given a certain amount of time to physically move to the new position. 

The TomServo library and class work by constantly iterating through all of the servos which have not yet reached their final target position. Each servo's position is incremented or decremented by 1, and the servo is given a short amount of time (~30ms) to physically move to the new position and then the `detach()` method is called in order to stop the PWM generation and lower the power consumption.

This iteration continues until all servos have been given time to reach their new target positions and they are all left in a detached state, consuming as little power as possible from a controlled servo.
