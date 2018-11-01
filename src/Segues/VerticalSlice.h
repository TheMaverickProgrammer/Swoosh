#pragma once
#include <Swoosh\Segue.h>
#include <Swoosh\Game.h>
#include <Swoosh\Ease.h>

using namespace swoosh;

class VerticalSlice : public Segue {
private:
  sf::Texture* temp;
  sf::Vector2u windowSize;
  int direction;

public:
  virtual void onDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = 1.0 - ease::bezierPopOut(elapsed, duration);

    this->drawLastActivity(surface);

    surface.display(); // flip and ready the buffer

    if (temp) delete temp;
    temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    sf::Sprite left(*temp); 
    left.setTextureRect(sf::IntRect(0, 0, windowSize.x/2.0, windowSize.y));
    left.setPosition(0, direction * alpha * left.getTexture()->getSize().y);

    sf::Sprite right(*temp);
    right.setTextureRect(sf::IntRect(windowSize.x/2.0, 0, windowSize.x, windowSize.y));
    right.setPosition(windowSize.x/2.0, direction * -alpha * right.getTexture()->getSize().y);

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

  VerticalSlice(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */ 
    temp = nullptr;
    windowSize = getController().getInitialWindowSize();
    direction = rand() % 2 == 0 ? -1 : 1;
  }

  virtual ~VerticalSlice() { if(temp) delete temp; }
};
