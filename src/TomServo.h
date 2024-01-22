/*\
|*| TomServo.h
|*|
|*|
\*/

#ifndef TOMSERVO_H_INCL
#define TOMSERVO_H_INCL

#include "Arduino.h"
#include "Servo.h"
#include <inttypes.h>
#include <ctype.h>

// set the minimum and maximum width of a servo pulse
static unsigned const MinWidth = 550;
static unsigned const MaxWidth = 2400;
static unsigned const DefaultPos = MinWidth + ((MaxWidth - MinWidth) / 2);

struct TomServo {
private:
    Servo       servo;          // the servo under control
    uint16_t    pin : 6,        // the pin # (0 - 63)
          completed : 1,        // motor is in position (and detached if enabled)
       allow_detach : 1,        // allow detachment when completed
          direction : 1;        // incr := 1, decr := 0
    uint16_t    min_width;      // min microseconds
    uint16_t    max_width;      // max microseconds
    uint16_t    pos;            // last written pos in microseconds
    uint16_t    delta;          // distance to move
    uint32_t    us_per_inc;     // time per incr or decr
    uint32_t    last_write;     // time of last write

    // no default ctor
    TomServo() = delete;

    // no copy ctor
    TomServo(TomServo&) = delete;

    // no assignment
    TomServo &operator = (TomServo &) = delete;

public:

    // constructor
    TomServo(signed const _pin,
             signed const _min = MinWidth,
             signed const _max = MaxWidth,
             signed const _pos = -1);

    // destructor
    virtual ~TomServo();

    //
    // Methods
    //

    // enable or disable detachment after moves have been completed
    bool enableDetachment(bool const allow);

    // move to the initial position. We must start at a known spot initially.
    void begin(uint32_t const _pos);

    // move immediately to a position
    void write(uint32_t _pos);

    // move smoothly to a position over time
    void write(uint16_t const _pos, uint32_t _dur);

    // give this servo a time slice
    bool update();

    // stop driving the output
    void detach();

    //
    // Attributes
    //

    // get the minimum pulse width in microseconds
    uint32_t minWidth() const;

    // get the maximum pulse width in microseconds
    uint32_t maxWidth() const;

    // see if the servo is attached (driving the output) or not
    bool     attached();

    // get the current pulse width in milliseconds
    uint32_t position() const;

    // see if the target position has been reached
    bool     complete() const;
};

#endif // #ifdef TOMSERVO_H_INCL
