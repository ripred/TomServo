# Thomas  Servo  Eskwire

![TomServo32x32.png](TomServo32x32.png)
### Sophisticated Servo Management for the Discriminating User.

The TomServo library lets you power multiple servos
from a single rechargeable battery.  It works by putting the
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
their position because they think the servo is ever in the "wrong"
position.

Example Use:
```
/* 
 * TomServoSweep.ino
 * 
 * Example sweep using two servos.
 * Each servo is swept back and forth using a specified amount of time.
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
}```
