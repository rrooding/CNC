#include "pendant_encoder.h"
#include <zephyr/logging/log.h>
#include <initializer_list>

LOG_MODULE_REGISTER(pendant_encoder);

PendantEncoder* PendantEncoder::instance { nullptr };

static void encoder_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {   
    if (!PendantEncoder::instance) {
        LOG_ERR("Encoder instance not available");
        return;
    }

    const int a { gpio_pin_get_dt(&PendantEncoder::instance->m_config.a) };
    const int b { gpio_pin_get_dt(&PendantEncoder::instance->m_config.b) };

    if (a != b) {
        PendantEncoder::instance->m_position += 1;  // Clockwise
    } else {
        PendantEncoder::instance->m_position -= 1;  // Counterclockwise
    }
}

PendantEncoder::PendantEncoder(const EncoderConfig& config) : m_position(0), m_config(config) {
    instance = this;

    for(const auto *pin : {&m_config.a, &m_config.b}) {
        if (!device_is_ready(pin->port) || gpio_pin_configure_dt(pin, GPIO_INPUT) < 0) {
            LOG_ERR("GPIO not ready or failed to configure encoder pin");
            k_fatal_halt(0);
        }

        if(gpio_pin_interrupt_configure_dt(pin, GPIO_INT_EDGE_BOTH) < 0) {
            LOG_ERR("Failed to configure encoder pin interrupt");
            k_fatal_halt(0);
        }
    }

    gpio_init_callback(&m_gpio_cb_a, encoder_isr, BIT(m_config.a.pin));
    gpio_add_callback(m_config.a.port, &m_gpio_cb_a);

    LOG_DBG("Pendant encoder initialized");
}

int PendantEncoder::get_position() const {
    return m_position;
}

void PendantEncoder::reset_position() {
    m_position = 0;
}
