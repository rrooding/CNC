#pragma once
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/pinctrl.h>
#include <string>

class UARTInterface {
public:
    struct UARTConfig {
        const struct device* dev;
        const struct pinctrl_dev_config* pinctrl;
    };

    UARTInterface(const UARTConfig& config);

    void send(std::string_view data) const;

private:
    const UARTConfig m_config;

    const uart_config m_uart_config {
        .baudrate = 115200,
        .parity = UART_CFG_PARITY_NONE,
        .stop_bits = UART_CFG_STOP_BITS_1,
        .data_bits = UART_CFG_DATA_BITS_8,
        .flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
    };
};       
