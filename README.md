# Thomas  Servo  Eskwire

![TomServo32x32.png](TomServo32x32.png)
### Sophisticated Servo Management for the Discriminating User.

The TomServo library lets you power multiple servos
from a single rechargeable battery.  By putting the
servo control pin into a high-z state when it is not 
being used we can disable the driving of the servo
and greatly reduce the current demands.

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
