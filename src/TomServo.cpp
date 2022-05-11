/*\
|*| TomServo.cpp
|*|
|*|
\*/

#include "TomServo.h"

// construction / destruction
TomServo::TomServo(signed const _pin,
          signed const _min /* = MinWidth */,
          signed const _max /* = MaxWidth */,
          signed const _pos /* = -1 */ ) :
        pin(_pin),
  completed(true),
allow_detach(true) {
    min_width = _min;
    max_width = _max;
    pos = (-1 == _pos) ? ((_max - _min) / 2) : DefaultPos;
}

TomServo::~TomServo() {
    if (servo.attached()) {
        servo.detach();
    }
}

//
// Methods
//


// enable or disable detachment after moves have been completed
bool TomServo::enableDetachment(bool const allow) {
    bool const prev = allow_detach;
    if ( (allow_detach = allow) ) {
        if (servo.attached() && complete()) {  // stay attached if not finished
            servo.detach();
        }
    }
    else {
        if (!servo.attached()) {
            servo.attach(pin, min_width, max_width);
        }
    }

    return prev;
}


// move to the initial position. We must start at a known spot initially.
void TomServo::begin(uint32_t const _pos) {
    servo.attach(pin, min_width, max_width);
    servo.write(_pos);
    delta = 0;
    delayMicroseconds((max_width - min_width) * 60);
    if (allow_detach) {
        servo.detach();
    }
}

// move immediately to a position
void TomServo::write(uint32_t _pos) {
    servo.write(pos = _pos);
}

// move smoothly to a position over time
void TomServo::write(uint16_t const _pos, uint32_t _dur) {
    // if we are already at this position return
    if (_pos == pos) { return; }

    completed = false;

    if (_pos < pos) {
        direction = 0;
        delta = pos - _pos;
    }
    else {
        direction = 1;
        delta = _pos - pos;
    }

    // keep us_per_inc >= 1
    if (_dur < delta) {
        _dur = delta;
    }

    us_per_inc = _dur / delta;

    if (allow_detach) {
        if (!servo.attached()) {
            servo.attach(pin, min_width, max_width);
        }
    }

    servo.write(pos + (direction ? 1 : -1));
    last_write = micros();
}


// give this servo a time slice
bool TomServo::update() {
    if (0 == delta && completed) { return true; }

    uint32_t now = micros();
    uint16_t steps = (now - last_write) / us_per_inc;
    if (0 == steps) {
        // not enough time has elapsed for motor to be at last written position
        return false;
    }

    // At least one step duration has elapsed so the servo
    // has had time to be at last written position.

    // See if we are done:
    if (0 == delta) {
        completed = true;
        if (allow_detach) {
            servo.detach();
        }
        return false;
    }

    //delta -= (steps > delta) ? delta : steps;
    delta -= min(steps, delta);
    if (direction) {
        pos += steps;
    }
    else {
        pos -= steps;
    }

    servo.write(pos);
    last_write = micros();

    return false;
}


// stop driving the output
void TomServo::detach() {
    if (servo.attached()) {
        servo.detach();
    }
}

//
// Attributes
//

// get the minimum pulse width in microseconds
uint32_t TomServo::minWidth() const {
    return min_width;
}

// get the maximum pulse width in microseconds
uint32_t TomServo::maxWidth() const {
    return max_width;
}

// see if the servo is attached (driving the output) or not
bool     TomServo::attached() {
    return servo.attached();
}

// get the current pulse width in milliseconds
uint32_t TomServo::position() const {
    return pos;
}

// see if the target position has been reached
bool     TomServo::complete() const {
    return completed;
}
