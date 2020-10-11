#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>

using namespace swoosh;

/**
  @class SlideIn
  @brief Slides the next scene over and on top of the current one
  @param direction. Compile-time constant that determines which direction to slide in to.

  Behavior is the same across all quality modes
*/

template<types::direction direction>
class SlideIn : public Segue {
public:

 void onDraw(sf::RenderTexture& surface) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);

    this->drawLastActivity(surface);

    surface.display(); // flip and ready the buffer

    sf::Texture temp(surface.getTexture()); // Make a copy of the source texture

    sf::Sprite left(temp); 

    int lr = 0;
    int ud = 0;

    if (direction == direction::left ) lr = -1;
    if (direction == direction::right) lr = 1;
    if (direction == direction::up   ) ud = -1;
    if (direction == direction::down ) ud = 1;

    surface.clear();

    this->drawNextActivity(surface);

    surface.display(); // flip and ready the buffer
    sf::Texture temp2(surface.getTexture());
    sf::Sprite right(temp2);

    right.setPosition((float)-lr * (1.0f-(float)alpha) * right.getTexture()->getSize().x, (float)-ud * (1.0f-(float)alpha) * right.getTexture()->getSize().y);

    surface.draw(left);
    surface.draw(right);
  }

  SlideIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) { 
    /* ... */ 
  }

  ~SlideIn() { ; }
};
