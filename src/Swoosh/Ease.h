#pragma once

namespace swoosh {
  namespace ease {
    static constexpr double pi = 3.14159265358979323846;
    
    template<typename T>
    static constexpr T radians(T degrees) { return (degrees * pi) / (T)180.0;  }

    template<typename T>
    static constexpr T interpolate(T factor, T a, T b) {
      return a + ((b - a) * factor);
    }

    template<typename T>
    static constexpr T linear(T delta, T length, T power) {
      T normal = 1.0 / length;

      T x = delta * normal;

      if (x >= 1) {
        x = 1;
      }

      T exponential = x;

      for (int i = 1; i < power; i++) {
        exponential *= exponential;
      }

      T y = exponential;

      return y;
    }
    
    /*
    y = (1 - abs(2-x*4) + 1)/2

    sharp back and forth, no easing
    */
    template<typename T>
    static constexpr T inOut(T delta, T length) {
      T normal = 1.0 / length;

      T x = delta * normal;

      if (x >= 1) {
        x = 1;
      }

      T y = (1.0 - std::abs(2.0 - x * 4.0) + 1.0) / 2.0;
      return y;
    }

    template<typename T>
    static constexpr T wideParabola(T delta, T length, T power) {
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

      for (int i = 1; i < power; i++) {
        exponential *= exponential;
      }

      T y = 1 - exponential;

      return y;
    }

    /*
      y = 3x ^ 2 - 2x ^ 4

      overshoot destination and slide back at the end
    */
    template<typename T>
    static constexpr T bezierPopIn(T delta, T length) {
      T normal = 1.0 / length;

      T x = delta * normal;

      if (x >= 1) {
        x = 1;
      }

    
      T part1 = 3.0 * x * x;
      T part2 = 2.0 * x * x * x * x;

      T y = part1 - part2;

      return y;
    }

    /*
    y = 3(1-x) ^ 2 - 2(1-x) ^ 4

    pop out and then slide out
  */
    template<typename T>
    static constexpr T bezierPopOut(T delta, T length) {
      T normal = 1.0 / length;

      T x = delta * normal;

      if (x >= 1) {
        x = 1;
      }

      x = (1.0 - x);
      T part1 = 3.0 * x * x;
      T part2 = 2.0 * x * x * x * x;

      T y = part1 - part2;

      return y;
    }
  }
}