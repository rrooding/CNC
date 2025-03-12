#pragma once
#include <optional>
#include <string>

class GCodeGenerator {
public:
    struct MotionParams {
        char axis { '0' };
        int scale_factor { 0 };
    };

    void set_params(MotionParams params) noexcept;

    std::optional<std::string> jog(int steps) const;

private:
    MotionParams m_params;

    static constexpr int feed_rate { 1000 };  // 1000mm/min
    static constexpr float step_size { 0.01f };  // 0.01mm per step

    float steps_to_scaled_mm(int steps) const noexcept;
};
