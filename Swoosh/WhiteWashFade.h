#pragma once
#include "Segue.h"

class WhiteWashFade : public Segue {
public:
  virtual void OnDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double x = elapsed / duration;
    double alpha = 1 - (x * x);

    if (elapsed <= duration * 0.5)
      this->DrawLastActivity(surface);
    else
      this->DrawNextActivity(surface);

    sf::RectangleShape whiteout;
    whiteout.setSize(sf::Vector2f(controller.getWindow().getSize().x, controller.getWindow().getSize().y));
    whiteout.setFillColor(sf::Color(255, 255, 255, alpha*255));
    surface.draw(whiteout);
  }

  WhiteWashFade(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) { /* ... */ }
  virtual ~WhiteWashFade() { ; }
};