#ifndef __TOMSERVO_MOTION_H__
#define __TOMSERVO_MOTION_H__

#include <Arduino.h>

typedef void (*tomservo_on_complete_cb_t)(void * ctx);

class TomServoMotion {

private:
    uint16_t completed        : 1;
    uint16_t direction        : 1;
    uint16_t cb_pending       : 1;

    uint16_t pos;
    uint16_t delta;

    uint32_t us_per_inc;
    uint32_t last_write;

    tomservo_on_complete_cb_t on_complete_cb;
    void * on_complete_ctx;

public:
    TomServoMotion(uint16_t const start_pos);

    void set_on_complete(tomservo_on_complete_cb_t const cb, void * const ctx);

    uint16_t position() const;
    bool complete() const;

    // begin()/write() immediate actions: update internal state only (no callback).
    void begin_at(uint16_t const new_pos, uint32_t const now_us);
    void write_immediate(uint16_t const new_pos, uint32_t const now_us);

    // Timed move scheduling. If target == current, marks complete and sets cb_pending.
    // Returns true if the move is already complete (delta == 0).
    bool schedule_move(uint16_t const target_pos, uint32_t const dur_us, uint32_t const now_us);

    // Advances motion timing and (optionally) yields a new position to apply to the backend.
    // did_write is true only when at least one step is applied and out_pos changes.
    // reached_target is true only on the call that consumes the final delta.
    // Returns true only when motion is already complete and no work is needed.
    bool update(uint32_t const now_us, bool & did_write, uint16_t & out_pos, bool & reached_target);

    // Completion callback is fired only for timed moves, and only once per move.
    bool callback_pending() const;
    void fire_callback_if_pending();
};

#endif // __TOMSERVO_MOTION_H__

