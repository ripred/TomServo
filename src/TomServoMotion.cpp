#include <Arduino.h>
#include "TomServoMotion.h"

TomServoMotion::TomServoMotion(uint16_t const start_pos)
    : completed(true),
      direction(0),
      cb_pending(0),
      pos(start_pos),
      delta(0),
      us_per_inc(0),
      last_write(0),
      on_complete_cb(NULL),
      on_complete_ctx(NULL) {
}

void TomServoMotion::set_on_complete(tomservo_on_complete_cb_t const cb, void * const ctx) {
    on_complete_cb = cb;
    on_complete_ctx = ctx;
}

uint16_t TomServoMotion::position() const {
    return pos;
}

bool TomServoMotion::complete() const {
    return completed;
}

void TomServoMotion::begin_at(uint16_t const new_pos, uint32_t const now_us) {
    pos = new_pos;
    delta = 0;
    us_per_inc = 0;
    completed = true;
    cb_pending = 0;
    last_write = now_us;
}

void TomServoMotion::write_immediate(uint16_t const new_pos, uint32_t const now_us) {
    pos = new_pos;
    delta = 0;
    us_per_inc = 0;
    completed = true;
    cb_pending = 0;
    last_write = now_us;
}

bool TomServoMotion::schedule_move(uint16_t const target_pos, uint32_t const dur_us, uint32_t const now_us) {

    completed = false;

    direction = (target_pos > pos);
    delta = direction ? uint16_t(target_pos - pos) : uint16_t(pos - target_pos);

    if (0 == delta) {
        completed = true;
        us_per_inc = 0;
        last_write = now_us;
        cb_pending = 1;
        return true;
    }

    us_per_inc = dur_us / delta;
    last_write = now_us;
    cb_pending = 0;

    return false;
}

bool TomServoMotion::update(uint32_t const now_us, bool & did_write, uint16_t & out_pos, bool & reached_target) {

    did_write = false;
    reached_target = false;
    out_pos = pos;

    if (0 == delta && completed) {
        return true;
    }

    if (0 == us_per_inc) {
        last_write = now_us;
        return complete();
    }

    uint32_t const elapsed = now_us - last_write;
    uint16_t const steps = uint16_t(elapsed / us_per_inc);

    if (0 == steps) {
        return false;
    }

    last_write += uint32_t(steps) * us_per_inc;

    uint16_t const step_count = (steps > delta) ? delta : steps;

    if (direction) {
        pos = uint16_t(pos + step_count);
    } else {
        pos = uint16_t(pos - step_count);
    }

    delta = uint16_t(delta - step_count);

    did_write = true;
    out_pos = pos;

    if (0 == delta) {
        completed = true;
        reached_target = true;
        cb_pending = 1;
        return false;
    }

    return false;
}

bool TomServoMotion::callback_pending() const {
    return (0 != cb_pending);
}

void TomServoMotion::fire_callback_if_pending() {

    if (0 == cb_pending) {
        return;
    }

    cb_pending = 0;

    if (NULL != on_complete_cb) {
        on_complete_cb(on_complete_ctx);
    }
}

