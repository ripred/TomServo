#ifndef __TOMSERVO_PCA9685_H__
#define __TOMSERVO_PCA9685_H__

#include <Arduino.h>
#include "TomServoMotion.h"
#include "TomServo.h"

// Forward declaration to avoid pulling in Adafruit headers unless this class is used.
class Adafruit_PWMServoDriver;

class TomServoPCA9685 {

private:
    Adafruit_PWMServoDriver * driver;
    uint8_t channel;

    uint16_t completed    : 1;
    uint16_t allow_detach : 1;
    uint16_t enabled      : 1;

    uint16_t min_width;
    uint16_t max_width;
    uint16_t pwm_hz;

    TomServoMotion motion;

    uint16_t angle_to_pulse_us(uint16_t const angle_deg) const;
    uint16_t pulse_us_to_ticks(uint16_t const pulse_us) const;

    void enable_output_for_angle(uint16_t const angle_deg);
    void disable_output();

    TomServoPCA9685();
    TomServoPCA9685(TomServoPCA9685 const &);
    TomServoPCA9685 & operator=(TomServoPCA9685 const &);

public:
    TomServoPCA9685(Adafruit_PWMServoDriver & pca,
                    uint8_t const _channel,
                    int const _min = MinWidth,
                    int const _max = MaxWidth,
                    int const _pos = -1,
                    uint16_t const _pwm_hz = 50);

    ~TomServoPCA9685();

    bool enableDetachment(bool const allow);

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

#endif // __TOMSERVO_PCA9685_H__

