#pragma once
#include <Swoosh\Segue.h>
#include <Swoosh\Game.h>
#include <Swoosh\Ease.h>

using namespace swoosh;

class VerticalOpen : public Segue {
private:
  sf::Texture* temp;
  sf::Vector2u windowSize;
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
    left.setTextureRect(sf::IntRect(0, 0, windowSize.x/2.0, windowSize.y));
    left.setPosition(-alpha * left.getTextureRect().width, 0);

    sf::Sprite right(*temp);
    right.setTextureRect(sf::IntRect(windowSize.x/2.0, 0, windowSize.x, windowSize.y));
    right.setPosition(windowSize.x/2.0 + (alpha * (right.getTextureRect().width-right.getTextureRect().left)), 0);

    surface.clear();

    this->drawNextActivity(surface);

    surface.display(); // flip and ready the buffer
    sf::Sprite next(surface.getTexture());

    sf::RenderWindow& window = getController().getWindow();
    window.draw(next);
    window.draw(left);
    window.draw(right);

    surface.clear(sf::Color::Transparent);
  }

  VerticalOpen(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */ 
    temp = nullptr;
    windowSize = getController().getInitialWindowSize();
  }

  virtual ~VerticalOpen() { if(temp) delete temp; }
};
