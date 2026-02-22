#include "D_solveWeighted.h"
#include <utility>
#include <cmath>
#include <iostream>
#include <random>
namespace fasm::math
{
    // ===== 通常最小二乗 =====
    std::vector<double> polyFit(const std::vector<std::pair<double, double>> &pts, int deg)
    {
        int n = deg;
        std::vector<std::vector<double>> ATA(n + 1, std::vector<double>(n + 1, 0));
        std::vector<double> ATb(n + 1, 0);

        for (auto &p : pts)
        {
            double x = p.first, y = p.second;

            std::vector<double> xp(n + 1, 1.0);
            for (int i = 1; i <= n; ++i)
                xp[i] = xp[i - 1] * x;

            for (int r = 0; r <= n; ++r)
            {
                for (int c = 0; c <= n; ++c)
                    ATA[r][c] += xp[r] * xp[c];

                ATb[r] += xp[r] * y;
            }
        }

        // Gauss
        for (int i = 0; i <= n; ++i)
        {
            int maxRow = i;
            for (int k = i + 1; k <= n; ++k)
                if (fabs(ATA[k][i]) > fabs(ATA[maxRow][i]))
                    maxRow = k;

            std::swap(ATA[i], ATA[maxRow]);
            std::swap(ATb[i], ATb[maxRow]);

            double pivot = ATA[i][i];

            for (int j = i; j <= n; ++j)
                ATA[i][j] /= pivot;
            ATb[i] /= pivot;

            for (int k = 0; k <= n; ++k)
            {
                if (k == i)
                    continue;
                double f = ATA[k][i];
                for (int j = i; j <= n; ++j)
                    ATA[k][j] -= f * ATA[i][j];
                ATb[k] -= f * ATb[i];
            }
        }

        return ATb;
    }

    // ===== Huberロバスト =====
    std::vector<double> robustFit(
        const std::vector<std::pair<double, double>> &pts,
        int deg, int iter, double delta)
    {
        int m = pts.size();
        std::vector<double> w(m, 1.0), coef;

        for (int t = 0; t < iter; ++t)
        {
            // weighted
            std::vector<std::vector<double>> ATA(deg + 1, std::vector<double>(deg + 1, 0));
            std::vector<double> ATb(deg + 1, 0);

            for (int i = 0; i < m; ++i)
            {
                double x = pts[i].first, y = pts[i].second;

                std::vector<double> xp(deg + 1, 1);
                for (int j = 1; j <= deg; ++j)
                    xp[j] = xp[j - 1] * x;

                for (int r = 0; r <= deg; ++r)
                {
                    for (int c = 0; c <= deg; ++c)
                        ATA[r][c] += w[i] * xp[r] * xp[c];

                    ATb[r] += w[i] * xp[r] * y;
                }
            }

            // solve
            for (int i = 0; i <= deg; ++i)
            {
                int maxRow = i;
                for (int k = i + 1; k <= deg; ++k)
                    if (fabs(ATA[k][i]) > fabs(ATA[maxRow][i]))
                        maxRow = k;

                std::swap(ATA[i], ATA[maxRow]);
                std::swap(ATb[i], ATb[maxRow]);

                double pivot = ATA[i][i];

                for (int j = i; j <= deg; ++j)
                    ATA[i][j] /= pivot;
                ATb[i] /= pivot;

                for (int k = 0; k <= deg; ++k)
                {
                    if (k == i)
                        continue;
                    double f = ATA[k][i];
                    for (int j = i; j <= deg; ++j)
                        ATA[k][j] -= f * ATA[i][j];
                    ATb[k] -= f * ATb[i];
                }
            }

            coef = ATb;

            // Huber update
            for (int i = 0; i < m; ++i)
            {
                double x = pts[i].first, y = pts[i].second;

                double fx = 0, xp = 1;
                for (auto c : coef)
                {
                    fx += c * xp;
                    xp *= x;
                }

                double e = fabs(y - fx);
                w[i] = (e <= delta) ? 1.0 : delta / e;
            }
        }

        return coef;
    }
}
