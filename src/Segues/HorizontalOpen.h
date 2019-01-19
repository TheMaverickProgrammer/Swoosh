#pragma once
#include <Swoosh\Segue.h>
#include <Swoosh\Game.h>
#include <Swoosh\Ease.h>

using namespace swoosh;

class HorizontalOpen : public Segue {
private:
  sf::Texture* temp;
  sf::Vector2u windowSize;
  int direction;
public:
  virtual void onDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);

    this->drawLastActivity(surface);

    surface.display(); // flip and ready the buffer

    if (temp) delete temp;
    temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    sf::Sprite top(*temp); 
    top.setTextureRect(sf::IntRect(0, 0, windowSize.x, (int)(windowSize.y / 2.0)));
    top.setPosition(0.0f, (float)(-alpha * top.getTextureRect().height));

    sf::Sprite bottom(*temp);
    bottom.setTextureRect(sf::IntRect(0, (int)(windowSize.y / 2.0), windowSize.x, windowSize.y));
    bottom.setPosition(0.0f, (float)(windowSize.y/2.0f) +  ((float)alpha * (bottom.getTextureRect().height-bottom.getTextureRect().top)));

    surface.clear();

    this->drawNextActivity(surface);

    surface.display(); // flip and ready the buffer
    sf::Sprite right(surface.getTexture());

    sf::RenderWindow& window = getController().getWindow();
    window.draw(right);
    window.draw(top);
    window.draw(bottom);

    surface.clear(sf::Color::Transparent);
  }

  HorizontalOpen(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */ 
    temp = nullptr;
    windowSize = getController().getVirtualWindowSize();
  }

  virtual ~HorizontalOpen() { if(temp) delete temp; }
};
