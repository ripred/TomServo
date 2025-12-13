#include <Arduino.h>
#include "TomServo.h"

TomServo::TomServo(int const _pin, int const _min, int const _max, int const _pos)
    : pin(_pin),
      completed(true),
      allow_detach(true),
      min_width(uint16_t(_min)),
      max_width(uint16_t(_max)),
      motion(uint16_t((-1 == _pos) ? DefaultPos : _pos)) {
}

TomServo::~TomServo() {
    servo.detach();
}

bool TomServo::enableDetachment(bool const allow) {
    allow_detach = allow;
    return allow_detach;
}

void TomServo::onComplete(tomservo_on_complete_cb_t const cb, void * const ctx) {
    motion.set_on_complete(cb, ctx);
}

void TomServo::begin(uint32_t const _pos) {

    uint32_t const now = micros();
    uint16_t const pos = uint16_t(_pos);

    motion.begin_at(pos, now);

    servo.attach(pin, min_width, max_width);
    servo.write(pos);

    delayMicroseconds((max_width - min_width) * 60);

    if (allow_detach) {
        servo.detach();
    }

    completed = true;
}

void TomServo::write(uint32_t const _pos) {

    uint32_t const now = micros();
    uint16_t const pos = uint16_t(_pos);

    motion.write_immediate(pos, now);

    servo.attach(pin, min_width, max_width);
    servo.write(pos);

    if (allow_detach) {
        servo.detach();
    }

    completed = true;
}

void TomServo::write(uint16_t const _pos, uint32_t const dur) {

    completed = false;

    if (allow_detach) {
        servo.attach(pin, min_width, max_width);
    } else if (!servo.attached()) {
        servo.attach(pin, min_width, max_width);
    }

    bool const already_complete = motion.schedule_move(_pos, dur, micros());

    if (already_complete) {
        completed = true;

        if (allow_detach) {
            servo.detach();
        }

        motion.fire_callback_if_pending();
    }
}

bool TomServo::update() {

    if (motion.complete() && completed) {
        return true;
    }

    bool did_write = false;
    bool reached_target = false;
    uint16_t new_pos = motion.position();

    bool const idle = motion.update(micros(), did_write, new_pos, reached_target);

    if (did_write) {
        servo.write(new_pos);
    }

    if (reached_target) {
        completed = true;

        if (allow_detach) {
            servo.detach();
        }

        motion.fire_callback_if_pending();
        return false;
    }

    return idle;
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
    return motion.position();
}

bool TomServo::complete() const {
    return completed;
}

