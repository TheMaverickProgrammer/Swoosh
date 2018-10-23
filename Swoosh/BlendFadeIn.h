#pragma once
#include "Segue.h"
#include "EaseFunctions.h"

class BlendFadeIn : public Segue {
private:
  sf::Texture* temp;
  int direction = 0;

public:
  virtual void OnDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1);

    this->DrawLastActivity(surface);

    surface.display(); // flip and ready the buffer

    if (temp) delete temp;
    temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    sf::Sprite left(*temp);

    surface.clear();

    this->DrawNextActivity(surface);

    surface.display(); // flip and ready the buffer
    sf::Sprite right(surface.getTexture());

    left.setColor(sf::Color(255, 255, 255, (1-alpha) * 255));
    right.setColor(sf::Color(255, 255, 255, alpha * 255));

    controller.getWindow().draw(left);
    controller.getWindow().draw(right);

    surface.clear(sf::Color::Transparent);
  }

  BlendFadeIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    temp = nullptr;
  }

  virtual ~BlendFadeIn() { ; }
};
