#include <Arduino.h>
#include "TomServo.h"

TomServo::TomServo(int const _pin, int const _min, int const _max, int const _pos)
    : pin(_pin), completed(true), allow_detach(true), direction(0) {

    min_width = _min;
    max_width = _max;

    // Position is in degrees. If no starting position is provided, use 90°.
    if (-1 == _pos) {
        pos = DefaultPos;
    } else {
        pos = uint16_t(_pos);
    }

    delta = 0;
    us_per_inc = 0;
    last_write = 0;
}

TomServo::~TomServo() {
    servo.detach();
}

bool TomServo::enableDetachment(bool const allow) {
    allow_detach = allow;
    return allow_detach;
}

void TomServo::begin(uint32_t const _pos) {

    pos = uint16_t(_pos);
    delta = 0;
    completed = true;

    servo.attach(pin, min_width, max_width);
    servo.write(pos);

    // Allow time for the servo to physically reach the starting position.
    // This is a rough upper bound based on pulse width range.
    delayMicroseconds((max_width - min_width) * 60);

    if (allow_detach) {
        servo.detach();
    }

    last_write = micros();
}

void TomServo::write(uint32_t const _pos) {

    pos = uint16_t(_pos);
    delta = 0;
    completed = true;

    servo.attach(pin, min_width, max_width);
    servo.write(pos);

    if (allow_detach) {
        servo.detach();
    }

    last_write = micros();
}

void TomServo::write(uint16_t const _pos, uint32_t const dur) {

    completed = false;

    if (allow_detach) {
        servo.attach(pin, min_width, max_width);
    }

    direction = (_pos > pos);
    delta = direction ? (_pos - pos) : (pos - _pos);

    if (0 == delta) {
        // No movement required; treat as an immediate completion.
        completed = true;
        if (allow_detach) {
            servo.detach();
        }
        return;
    }

    // dur is the total time for delta degrees of motion.
    // us_per_inc is time per 1-degree increment.
    us_per_inc = dur / delta;
    last_write = micros();
}

bool TomServo::update() {

    if (0 == delta && completed) {
        return true;
    }

    uint32_t const now = micros();

    if (0 == us_per_inc) {
        last_write = now;
        return complete();
    }

    uint32_t const elapsed = now - last_write;
    uint16_t const steps = elapsed / us_per_inc;

    if (0 == steps) {
        return false;
    }

    last_write += uint32_t(steps) * us_per_inc;

    // Do not move farther than the remaining delta.
    uint16_t const step_count = (steps > delta) ? delta : steps;

    if (direction) {
        pos += step_count;
    } else {
        pos -= step_count;
    }

    delta -= step_count;

    servo.write(pos);

    if (0 == delta) {
        completed = true;
        if (allow_detach) {
            servo.detach();
        }
        return false;
    }

    return false;
}

void TomServo::detach() {
    servo.detach();
}

uint32_t TomServo::minWidth() const {
    return min_width;
}

uint32_t TomServo::maxWidth() const {
    return max_width;
}

bool TomServo::attached() {
    return servo.attached();
}

uint32_t TomServo::position() const {
    return pos;
}

bool TomServo::complete() const {
    return completed;
}

