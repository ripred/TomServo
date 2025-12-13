#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "TomServoPCA9685.h"

TomServoPCA9685::TomServoPCA9685(Adafruit_PWMServoDriver & pca,
                                 uint8_t const _channel,
                                 int const _min,
                                 int const _max,
                                 int const _pos,
                                 uint16_t const _pwm_hz)
    : driver(&pca),
      channel(_channel),
      completed(true),
      allow_detach(true),
      enabled(false),
      min_width(uint16_t(_min)),
      max_width(uint16_t(_max)),
      pwm_hz(_pwm_hz),
      motion(uint16_t((-1 == _pos) ? DefaultPos : _pos)) {

    if (15 < channel) {
        channel = 0;
    }

    if (0 == pwm_hz) {
        pwm_hz = 50;
    }
}

TomServoPCA9685::~TomServoPCA9685() {
    disable_output();
}

bool TomServoPCA9685::enableDetachment(bool const allow) {
    allow_detach = allow;
    return allow_detach;
}

void TomServoPCA9685::onComplete(tomservo_on_complete_cb_t const cb, void * const ctx) {
    motion.set_on_complete(cb, ctx);
}

uint16_t TomServoPCA9685::angle_to_pulse_us(uint16_t const angle_deg) const {

    uint16_t angle = angle_deg;
    if (180 < angle) {
        angle = 180;
    }

    uint32_t const range = uint32_t(max_width - min_width);
    uint32_t const pulse = uint32_t(min_width) + (uint32_t(angle) * range) / 180UL;

    return uint16_t(pulse);
}

uint16_t TomServoPCA9685::pulse_us_to_ticks(uint16_t const pulse_us) const {

    // ticks = pulse_us * (pwm_hz * 4096) / 1,000,000
    uint64_t const numer = uint64_t(pulse_us) * uint64_t(pwm_hz) * 4096ULL;
    uint64_t const ticks = (numer + 500000ULL) / 1000000ULL;

    if (4095ULL < ticks) {
        return 4095;
    }

    return uint16_t(ticks);
}

void TomServoPCA9685::enable_output_for_angle(uint16_t const angle_deg) {

    uint16_t const pulse_us = angle_to_pulse_us(angle_deg);
    uint16_t const ticks = pulse_us_to_ticks(pulse_us);

    // Valid servo waveform: ON at 0, OFF at pulse width.
    driver->setPWM(channel, 0, ticks);
    enabled = true;
}

void TomServoPCA9685::disable_output() {

    // Emulate detach: constant LOW / no valid servo frames.
    driver->setPWM(channel, 0, 0);
    enabled = false;
}

void TomServoPCA9685::begin(uint32_t const _pos) {

    uint32_t const now = micros();
    uint16_t const pos = uint16_t(_pos);

    motion.begin_at(pos, now);

    enable_output_for_angle(pos);

    // Match TomServo's rough settling delay.
    delayMicroseconds((max_width - min_width) * 60);

    if (allow_detach) {
        disable_output();
    }

    completed = true;
}

void TomServoPCA9685::write(uint32_t const _pos) {

    uint32_t const now = micros();
    uint16_t const pos = uint16_t(_pos);

    motion.write_immediate(pos, now);

    enable_output_for_angle(pos);

    if (allow_detach) {
        disable_output();
    }

    completed = true;
}

void TomServoPCA9685::write(uint16_t const _pos, uint32_t const dur) {

    completed = false;

    // Ensure valid waveform during motion.
    enable_output_for_angle(motion.position());

    bool const already_complete = motion.schedule_move(_pos, dur, micros());

    if (already_complete) {
        completed = true;

        if (allow_detach) {
            disable_output();
        } else {
            enable_output_for_angle(motion.position());
        }

        motion.fire_callback_if_pending();
    }
}

bool TomServoPCA9685::update() {

    if (motion.complete() && completed) {
        return true;
    }

    bool did_write = false;
    bool reached_target = false;
    uint16_t new_pos = motion.position();

    bool const idle = motion.update(micros(), did_write, new_pos, reached_target);

    if (did_write) {
        enable_output_for_angle(new_pos);
    }

    if (reached_target) {
        completed = true;

        if (allow_detach) {
            disable_output();
        } else {
            enable_output_for_angle(motion.position());
        }

        motion.fire_callback_if_pending();
        return false;
    }

    return idle;
}

void TomServoPCA9685::detach() {
    disable_output();
}

bool TomServoPCA9685::attached() {
    return enabled;
}

uint32_t TomServoPCA9685::minWidth() const {
    return min_width;
}

uint32_t TomServoPCA9685::maxWidth() const {
    return max_width;
}

uint32_t TomServoPCA9685::position() const {
    return motion.position();
}

bool TomServoPCA9685::complete() const {
    return completed;
}

