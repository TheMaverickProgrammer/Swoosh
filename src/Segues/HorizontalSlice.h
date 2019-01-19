#pragma once
#include <Swoosh\Segue.h>
#include <Swoosh\Game.h>
#include <Swoosh\Ease.h>

using namespace swoosh;

class HorizontalSlice : public Segue {
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

    sf::Sprite top(*temp); 
    top.setTextureRect(sf::IntRect(0, 0, windowSize.x, windowSize.y / 2));
    top.setPosition((float)(direction * alpha * top.getTexture()->getSize().x), 0.0f);

    sf::Sprite bottom(*temp);
    bottom.setTextureRect(sf::IntRect(0, windowSize.y / 2, windowSize.x, windowSize.y));
    bottom.setPosition((float)(direction * -alpha * bottom.getTexture()->getSize().x), (float)(windowSize.y/2.0f));

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

  HorizontalSlice(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */ 
    temp = nullptr;
    windowSize = getController().getVirtualWindowSize();
    direction = rand() % 2 == 0 ? -1 : 1;
  }

  virtual ~HorizontalSlice() { if(temp) delete temp; }
};
