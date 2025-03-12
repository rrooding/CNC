#pragma once
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

class PendantEncoder {
public:
    struct EncoderConfig {
        gpio_dt_spec a;
        gpio_dt_spec b;
    };

    PendantEncoder(const EncoderConfig& config);

    int get_position() const;
    void reset_position();

    static PendantEncoder* instance;

    volatile int m_position;
    EncoderConfig m_config;

private:
    struct gpio_callback m_gpio_cb_a;
    struct gpio_callback m_gpio_cb_b;
};
