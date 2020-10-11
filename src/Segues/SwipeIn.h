#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Game.h>
#include <Swoosh/Ease.h>

using namespace swoosh;


/**
  @class SwipeIn
  @brief The next scene is masked and unwrapped on-top of the current one like a swipe effect
  @param direction. Compile-time constant that determines which direction the next scene will swipe to.
  Behavior is the same across all quality modes
*/

template<types::direction direction>
class SwipeIn : public Segue {
private:
  sf::Vector2u windowSize;
public:
 void onDraw(sf::RenderTexture& surface) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);

    surface.clear(this->getLastActivityBGColor());
    this->drawLastActivity(surface);

    surface.display(); // flip and ready the buffer

    sf::Texture temp(surface.getTexture()); // Make a copy of the source texture

    sf::Sprite bottom(temp); 
    surface.clear();

    surface.clear(this->getLastActivityBGColor());
    this->drawNextActivity(surface);

    surface.display(); // flip and ready the buffer
    sf::Texture temp2(surface.getTexture());
    sf::Sprite top(temp2);

    int l = 0;
    int r = 0;
    int u = 0;
    int d = 0;

    // left
    if (direction == direction::left) {
      r = bottom.getTexture()->getSize().x;
      l = (int)(r - (alpha * (double)r));
      u = 0;
      d = bottom.getTexture()->getSize().y;


      top.setPosition((float)l, 0);
    }

    // right
    if (direction == direction::right) {
      r = (int)((double)bottom.getTexture()->getSize().x * alpha);
      l = 0;
      u = 0;
      d = bottom.getTexture()->getSize().y;
    }

    // up
    if (direction == direction::up) {
      r = bottom.getTexture()->getSize().x;
      l = 0;
      d = bottom.getTexture()->getSize().y;
      u = (int)((double)d - (alpha * (double)d));

      top.setPosition(0, (float)u);
    }

    // down 
    if (direction == direction::down) {
      r = bottom.getTexture()->getSize().x;
      l = 0;
      u = 0;
      d = (int)((double)bottom.getTexture()->getSize().y * alpha);
    }

    top.setTextureRect(sf::IntRect(l, u, r, d));

    surface.clear();
    surface.draw(bottom);
    surface.draw(top);
  }

  SwipeIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */ 
    windowSize = getController().getVirtualWindowSize();
  }

  ~SwipeIn() {; }
};
