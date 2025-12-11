#ifndef __TOMSERVO_H__
#define __TOMSERVO_H__

#include <Arduino.h>
#include <Servo.h>

static uint16_t const MinWidth  = 544;
static uint16_t const MaxWidth  = 2400;
static uint16_t const DefaultPos = 90;  // default angle in degrees

class TomServo {

private:
    Servo servo;

    uint16_t pin          : 6;
    uint16_t completed    : 1;
    uint16_t allow_detach : 1;
    uint16_t direction    : 1;

    uint16_t min_width;
    uint16_t max_width;

    // Current servo position in degrees (same units as Servo::write()).
    uint16_t pos;

    // Remaining distance to target position, in degrees.
    uint16_t delta;

    // Time per 1-degree increment, in microseconds.
    uint32_t us_per_inc;

    // Last time we advanced the servo, from micros().
    uint32_t last_write;

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

    // Attach, move immediately to _pos (degrees), wait for motion to complete,
    // then detach if auto-detach is enabled.
    void begin(uint32_t const _pos);

    // Immediate move to _pos (degrees). Attach, write, and then detach if
    // auto-detach is enabled.
    void write(uint32_t const _pos);

    // Timed move from current position to _pos (degrees) over dur microseconds.
    // Actual motion is performed incrementally in update().
    void write(uint16_t const _pos, uint32_t const dur);

    // Advance motion based on elapsed time since last_write.
    // Returns true only when motion is already complete and no work is needed.
    bool update();

    void detach();
    bool attached();

    uint32_t minWidth() const;
    uint32_t maxWidth() const;

    // Get the current servo position in degrees (same units as Servo::write()).
    uint32_t position() const;

    // True if the current scheduled motion has completed.
    bool complete() const;
};

#endif // __TOMSERVO_H__

