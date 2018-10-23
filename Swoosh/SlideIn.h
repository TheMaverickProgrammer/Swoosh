#pragma once
#include "Segue.h"
#include "EaseFunctions.h"

class SlideIn : public Segue {
private:
  sf::Texture* temp;

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
    left.setPosition(-alpha * left.getTexture()->getSize().x, 0);

    surface.clear();

    this->DrawNextActivity(surface);

    surface.display(); // flip and ready the buffer
    sf::Sprite right(surface.getTexture());

    left.setPosition(-alpha * left.getTexture()->getSize().x, 0);
    right.setPosition((1-alpha) * right.getTexture()->getSize().x, 0);

    controller.getWindow().draw(left);
    controller.getWindow().draw(right);

    surface.clear(sf::Color::Transparent);
  }

  SlideIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) { 
    /* ... */ 
    temp = nullptr;
  }

  virtual ~SlideIn() { ; }
};
