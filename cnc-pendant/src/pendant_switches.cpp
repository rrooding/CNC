#include "pendant_switches.h"
#include <zephyr/logging/log.h>
#include <initializer_list>

LOG_MODULE_REGISTER(pendant_switches);

PendantSwitches::PendantSwitches(const SwitchConfig& config) : m_config(config) {
    for(const auto *pin : {&m_config.axis_x, &m_config.axis_y, &m_config.axis_z, &m_config.scale_x1, &m_config.scale_x10, &m_config.scale_x100 }) {
        if (!device_is_ready(pin->port) || gpio_pin_configure_dt(pin, GPIO_INPUT | GPIO_PULL_UP) < 0) {
            LOG_ERR("Failed to configure switch pin");
            k_fatal_halt(0);
        }
    }

    LOG_DBG("Pendant switches initialized");
}

std::optional<char> PendantSwitches::get_axis() {
    const int x_state { gpio_pin_get_dt(&m_config.axis_x) };
    const int y_state { gpio_pin_get_dt(&m_config.axis_y) };
    const int z_state { gpio_pin_get_dt(&m_config.axis_z) };

    if (x_state < 0 || y_state < 0 || z_state < 0) {
        LOG_ERR("Failed to read axis GPIOs");
        return std::nullopt;  // Default to off on error
    }

    if (x_state == 0) {
        return 'X';
    } else if (y_state == 0) {
        return 'Y';
    } else if (z_state == 0) {
        return 'Z';
    } else {
        return std::nullopt;  // Off position or pendant not active (no GPIOs active)
    }
}

std::optional<int> PendantSwitches::get_scale_factor() {
    const int x1_state { gpio_pin_get_dt(&m_config.scale_x1) };
    const int x10_state { gpio_pin_get_dt(&m_config.scale_x10) };
    const int x100_state { gpio_pin_get_dt(&m_config.scale_x100) };

    if (x1_state < 0 || x10_state < 0 || x100_state < 0) {
        LOG_ERR("Failed to read scale factor GPIOs");
        return std::nullopt;
    }

    if (x1_state == 0) {
        return 1;
    } else if (x10_state == 0) {
        return 10;
    } else if (x100_state == 0) {
        return 100;
    } else {
        return std::nullopt;  // Pendant not active (no GPIOs active)
    }
}
