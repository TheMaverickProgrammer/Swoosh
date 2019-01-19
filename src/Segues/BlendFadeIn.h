#pragma once
#include <Swoosh\Segue.h>
#include <Swoosh\Ease.h>

using namespace swoosh;

class BlendFadeIn : public Segue {
private:
  sf::Texture* temp;
  int direction = 0;

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

    surface.clear();

    this->drawNextActivity(surface);

    surface.display(); // flip and ready the buffer
    sf::Sprite right(surface.getTexture());

    left.setColor(sf::Color(255, 255, 255, (sf::Uint8)((1.0-alpha) * 255.0)));
    right.setColor(sf::Color(255, 255, 255, (sf::Uint8)(alpha * 255.0)));

    getController().getWindow().draw(left);
    getController().getWindow().draw(right);

    surface.clear(sf::Color::Transparent);
  }

  BlendFadeIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    temp = nullptr;
  }

  virtual ~BlendFadeIn() { ; }
};
