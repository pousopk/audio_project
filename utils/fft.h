#pragma once

#include <vector>
#include <complex>
#include <cmath>

namespace FFT {
    constexpr float PI = 3.14159265358979323846f;

    // Basic Radix-2 Cooley-Tukey FFT
    inline void transform(std::vector<std::complex<float>>& data) {
        const size_t N = data.size();
        if (N <= 1) return;

        // Bit-reversal permutation
        for (size_t i = 1, j = 0; i < N; i++) {
            size_t bit = N >> 1;
            for (; j & bit; bit >>= 1) {
                j ^= bit;
            }
            j ^= bit;
            if (i < j) {
                std::swap(data[i], data[j]);
            }
        }

        // Iterative FFT
        for (size_t len = 2; len <= N; len <<= 1) {
            float ang = -2.0f * PI / len;
            std::complex<float> wlen(cos(ang), sin(ang));
            for (size_t i = 0; i < N; i += len) {
                std::complex<float> w(1);
                for (size_t j = 0; j < len / 2; j++) {
                    std::complex<float> u = data[i + j];
                    std::complex<float> v = data[i + j + len / 2] * w;
                    data[i + j] = u + v;
                    data[i + j + len / 2] = u - v;
                    w *= wlen;
                }
            }
        }
    }
}