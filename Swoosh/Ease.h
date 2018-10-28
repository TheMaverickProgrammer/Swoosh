#pragma once

namespace swoosh {
  namespace ease {
    double linear(double delta, double length, double power) {
      double normal = 1.0 / length;

      double x = delta * normal;

      if (x >= 1) {
        x = 1;
      }

      double exponential = x;

      for (int i = 0; i < power; i++) {
        exponential *= exponential;
      }

      double y = exponential;

      return y;
    }

    double wideParabola(double delta, double length, double power) {
      double normal = 2.0 / length;

      // Convert seconds elapsed to x values of 0 -> 2
      double x = delta * normal;

      // When x = 2, the parabola drops into the negatives
      // prevent that
      if (x >= 2) {
        x = 2;
      }

      // y = 1 - (x ^ 2 - 2x + 1) ^ n
      double poly = (x*x) - (2 * x) + 1;
      double exponential = poly;

      for (int i = 0; i < power; i++) {
        exponential *= exponential;
      }

      double y = 1 - exponential;

      return y;
    }
  }
}