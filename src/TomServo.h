#ifndef __TOMSERVO_H__
#define __TOMSERVO_H__

#include <Arduino.h>
#include <Servo.h>
#include "TomServoMotion.h"

static uint16_t const MinWidth  = 544;
static uint16_t const MaxWidth  = 2400;
static uint16_t const DefaultPos = 90;  // default angle in degrees

class TomServo {

private:
    Servo servo;

    uint16_t pin          : 6;
    uint16_t completed    : 1;
    uint16_t allow_detach : 1;

    uint16_t min_width;
    uint16_t max_width;

    TomServoMotion motion;

    TomServo();
    TomServo(TomServo const &);
    TomServo & operator=(TomServo const &);

public:
    TomServo(int const _pin,
             int const _min = MinWidth,
             int const _max = MaxWidth,
             int const _pos = -1);

    ~TomServo();

    // Enable or disable automatic detach when motion completes.
    bool enableDetachment(bool const allow);

    // Register a callback for timed moves (write(pos, dur)).
    // Called once when the servo reaches its target.
    void onComplete(tomservo_on_complete_cb_t const cb, void * const ctx);

    void begin(uint32_t const _pos);

    void write(uint32_t const _pos);

    void write(uint16_t const _pos, uint32_t const dur);

    bool update();

    void detach();
    bool attached();

    uint32_t minWidth() const;
    uint32_t maxWidth() const;

    uint32_t position() const;

    bool complete() const;
};

#endif // __TOMSERVO_H__

