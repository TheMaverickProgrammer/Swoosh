#pragma once
#include "Segue.h"
#include "EaseFunctions.h"

class SlideIn : public Segue {
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

    int lr = 0;
    int ud = 0;

    if (direction == 0) lr = -1;
    if (direction == 1) ud = -1;
    if (direction == 2) lr = 1;
    if (direction == 3) ud = 1;

    left.setPosition(lr * alpha * left.getTexture()->getSize().x, ud * alpha * left.getTexture()->getSize().y);

    surface.clear();

    this->DrawNextActivity(surface);

    surface.display(); // flip and ready the buffer
    sf::Sprite right(surface.getTexture());

    right.setPosition(-lr * (1-alpha) * right.getTexture()->getSize().x, -ud * (1-alpha) * right.getTexture()->getSize().y);

    controller.getWindow().draw(left);
    controller.getWindow().draw(right);

    surface.clear(sf::Color::Transparent);
  }

  SlideIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) { 
    /* ... */ 
    temp = nullptr;
    direction = rand() % 4; // Choose a random direction
  }

  virtual ~SlideIn() { ; }
};
