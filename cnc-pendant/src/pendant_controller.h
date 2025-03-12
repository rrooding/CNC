#pragma once
#include "gcode_generator.h"
#include "pendant_switches.h"
#include "pendant_encoder.h"
#include "uart_interface.h"

class PendantController {
public:
    PendantController(PendantSwitches& switches, PendantEncoder& encoder, UARTInterface& uart);

    void run();

private:
    GCodeGenerator m_gcode_generator{};
    PendantSwitches& m_switches;
    PendantEncoder& m_encoder;
    UARTInterface& m_uart;
};
