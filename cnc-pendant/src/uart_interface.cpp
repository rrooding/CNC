#include "uart_interface.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(uart_interface);

UARTInterface::UARTInterface(const UARTConfig& config) : m_config(config) {
    if (!device_is_ready(m_config.dev)) {
        LOG_ERR("UART device not ready");
        k_fatal_halt(0);
    }

    if (pinctrl_apply_state(m_config.pinctrl, PINCTRL_STATE_DEFAULT) < 0 ||
        uart_configure(m_config.dev, &m_uart_config) < 0) {
        LOG_ERR("Failed to configure UART");
        k_fatal_halt(0);
    }

    LOG_DBG("UART interface initialized");
}

void UARTInterface::send(std::string_view data) const {
    for (char c : data) {
        uart_poll_out(m_config.dev, static_cast<unsigned char>(c));
    }
}
