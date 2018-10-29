#pragma once

namespace swoosh {
  namespace ease {
    static constexpr double pi = 3.14159265358979323846;
    
    template<typename T>
    static constexpr T radians(T degrees) { return (degrees * pi) / 180.0;  }

    template<typename T>
    static constexpr T interpolate(T factor, T a, T b) {
      return a * (1.0 - factor) + b * factor;
    }

    template<typename T>
    static constexpr double linear(T delta, T length, T power) {
      T normal = 1.0 / length;

      T x = delta * normal;

      if (x >= 1) {
        x = 1;
      }

      T exponential = x;

      for (int i = 0; i < power; i++) {
        exponential *= exponential;
      }

      T y = exponential;

      return y;
    }

    template<typename T>
    static double constexpr wideParabola(T delta, T length, T power) {
      T normal = 2.0 / length;

      // Convert seconds elapsed to x values of 0 -> 2
      T x = delta * normal;

      // When x = 2, the parabola drops into the negatives
      // prevent that
      if (x >= 2) {
        x = 2;
      }

      // y = 1 - (x ^ 2 - 2x + 1) ^ n
      T poly = (x*x) - (2 * x) + 1;
      T exponential = poly;

      for (int i = 0; i < power; i++) {
        exponential *= exponential;
      }

      T y = 1 - exponential;

      return y;
    }
  }
}