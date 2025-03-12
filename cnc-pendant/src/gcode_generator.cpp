#include "gcode_generator.h"
#include <sstream>

void GCodeGenerator::set_params(MotionParams params) {
    m_params = params;
}

std::optional<std::string> GCodeGenerator::jog(int steps) const {
    if (m_params.axis != 'X' && m_params.axis != 'Y' && m_params.axis != 'Z') {
        // Invalid axis
        return std::nullopt;
    }

    if (steps == 0 || m_params.scale_factor <= 0) {
        // No movement or invalid scale
        return std::nullopt;
    }

    std::stringstream command{};
    command.precision(3);  // 3 decimal places

    command << "$J="  // GRBL jog command
            << "G91 "  // Relative positioning
            << "G21 "  // Millimeters
            << m_params.axis << steps_to_scaled_mm(steps)  // Scaled distance
            << " F" << feed_rate;  // Feed rate

    return command.str();
}

float GCodeGenerator::steps_to_scaled_mm(int steps) const noexcept {
    return step_size * steps * m_params.scale_factor;
}
