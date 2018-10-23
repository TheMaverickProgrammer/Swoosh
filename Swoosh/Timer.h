#pragma once
#include <SFML/System.hpp>

class Timer {
  sf::Clock clock;
  float elapsed;
  bool isPaused;

public:
  Timer() {
    isPaused = true;
    elapsed = 0;
    clock.restart();
  }

  void Reset() {
    clock.restart();
    elapsed = 0;
    isPaused = false;
  }

  void Start() {
    if (isPaused) {
      clock.restart();
    }
    isPaused = false;
  }

  void Pause() {
    if (!isPaused) {
      elapsed += clock.getElapsedTime().asMilliseconds();
    }
    isPaused = true;
  }

  bool IsPaused() { return isPaused; }

  float GetElapsed() {
    if (!isPaused) {
      return elapsed + clock.getElapsedTime().asMilliseconds();
    }
    return elapsed;
  }
};