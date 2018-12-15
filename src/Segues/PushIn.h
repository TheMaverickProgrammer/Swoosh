#pragma once
#include <Swoosh\Segue.h>
#include <Swoosh\Ease.h>

using namespace swoosh;

template<int direction>
class PushIn : public Segue {
private:
  sf::Texture* temp;

public:

  virtual void onDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);

    this->drawLastActivity(surface);

    surface.display(); // flip and ready the buffer

    if (temp) delete temp;
    temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    sf::Sprite left(*temp); 

    int lr = 0;
    int ud = 0;

    if (direction == 0) lr = -1;
    if (direction == 1) lr = 1;
    if (direction == 2) ud = -1;
    if (direction == 3) ud = 1;

    left.setPosition(lr * alpha * left.getTexture()->getSize().x, ud * alpha * left.getTexture()->getSize().y);

    surface.clear();

    this->drawNextActivity(surface);

    surface.display(); // flip and ready the buffer
    sf::Sprite right(surface.getTexture());

    right.setPosition(-lr * (1-alpha) * right.getTexture()->getSize().x, -ud * (1-alpha) * right.getTexture()->getSize().y);

    sf::RenderWindow& window = getController().getWindow();
    window.draw(left);
    window.draw(right);

    surface.clear(sf::Color::Transparent);
  }

  PushIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */ 
    temp = nullptr;
  }

  virtual ~PushIn() { ; }
};
