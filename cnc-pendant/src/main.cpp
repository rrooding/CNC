#include "pendant_controller.h"
#include "pendant_switches.h"
#include "pendant_encoder.h"
#include "uart_interface.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(cnc_pendant);

#define UART_DEVICE DT_NODELABEL(uart1)
PINCTRL_DT_DEFINE(UART_DEVICE);

static const UARTInterface::UARTConfig uart_config = {
    .dev = DEVICE_DT_GET(UART_DEVICE),
    .pinctrl = PINCTRL_DT_DEV_CONFIG_GET(UART_DEVICE),
};

static const PendantSwitches::SwitchConfig switch_config = {
    .axis_x = GPIO_DT_SPEC_GET(DT_PATH(pendant_switches, axis_x), gpios),
    .axis_y = GPIO_DT_SPEC_GET(DT_PATH(pendant_switches, axis_y), gpios),
    .axis_z = GPIO_DT_SPEC_GET(DT_PATH(pendant_switches, axis_z), gpios),
    .scale_x1 = GPIO_DT_SPEC_GET(DT_PATH(pendant_switches, scale_x1), gpios),
    .scale_x10 = GPIO_DT_SPEC_GET(DT_PATH(pendant_switches, scale_x10), gpios),
    .scale_x100 = GPIO_DT_SPEC_GET(DT_PATH(pendant_switches, scale_x100), gpios),
};

static const PendantEncoder::EncoderConfig encoder_config = {
    .a = GPIO_DT_SPEC_GET(DT_NODELABEL(pendant_encoder), a_gpios),
    .b = GPIO_DT_SPEC_GET(DT_NODELABEL(pendant_encoder), b_gpios),
};

int main() {
    LOG_INF("CNC Pendant (%s)", CONFIG_BOARD_TARGET);

    PendantSwitches switches(switch_config);
    PendantEncoder encoder(encoder_config);
    UARTInterface uart(uart_config);

    PendantController controller(switches, encoder, uart);
    controller.run();

    return 0;
}
