#pragma once
#include <vector>
namespace fasm::math
{
    std::vector<double> polyFit(const std::vector<std::pair<double, double>> &pts, int deg);

    // ===== Huberロバスト =====
    std::vector<double> robustFit(
        const std::vector<std::pair<double, double>> &pts,
        int deg, int iter = 20, double delta = 1.0);
}
