# TomServo

![TomServo32x32.png](TomServo32x32.png) Sophisticated Microprocessor Controlled Servo Management Library

The TomServo library gives you more sophisticated control over 
servo motors as well as offering power saving techniques to 
greatly reduce the amount of power required to use multiple 
servos in a project.  Multiple servo motors can be powered from 
batteries alone, a large departure from other servo interfaces.
 
By placing the output pin for the servo into a high-z 
(high impedence) state we stop the servo motor electronics
from driving the servo motor, greatly reducing the current flow
for that motor.  Depending on the nominal load and torque 
that the motor is under it is usually possible for the motor 
to remain stationary without being powered due to the gearing
ratios of the servo motor.  This feature can be turned on or off
as necessary for each individual servo in your project.
