#include "pendant_controller.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pendant_controller);

PendantController::PendantController(PendantSwitches& switches, PendantEncoder& encoder, UARTInterface& uart) : m_switches(switches), m_encoder(encoder), m_uart(uart) {}

void PendantController::run() {
    LOG_INF("Starting pendant controller...");

    while(true) {
        const auto axis { m_switches.get_axis() };
        const auto scale_factor { m_switches.get_scale_factor() };
        const int pos { m_encoder.get_position() };

        if (pos && axis) {
            m_gcode_generator.set_params({axis.value(), scale_factor.value()});
            const auto gcode{ m_gcode_generator.jog(pos) };

            if (gcode.has_value())
                m_uart.send(gcode.value() + '\n');
        }

        // Always reset the encoder position to prevent accumulation when the
        // pendant is not activated with the side button
        m_encoder.reset_position(); 

        k_msleep(20);
    }
}
