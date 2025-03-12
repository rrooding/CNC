#pragma once
#include <zephyr/drivers/gpio.h>
#include <optional>

class PendantSwitches {
public:
    struct SwitchConfig {
        gpio_dt_spec axis_x;
        gpio_dt_spec axis_y;
        gpio_dt_spec axis_z;

        gpio_dt_spec scale_x1;
        gpio_dt_spec scale_x10;
        gpio_dt_spec scale_x100;   
    };

    PendantSwitches(const SwitchConfig& config);

    std::optional<char> get_axis();
    std::optional<int> get_scale_factor();

private:
    const SwitchConfig m_config;
};
