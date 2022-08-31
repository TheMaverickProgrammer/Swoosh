#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Game.h>
#include <Swoosh/Ease.h>

using namespace swoosh;

/**
  @class ZoomIn
  @brief scales the last scene down into the center of the screen, revealing the next scene behind it
  
  If requested quality is set to mobile, capture the first screen and do not capture real-time.
 */

class ZoomOut : public Segue {
private:
  sf::Vector2u windowSize;
  sf::Texture next, last;
  bool firstPass{ true };
public:
 void onDraw(IRenderer& renderer) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::bezierPopOut(elapsed, duration);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;

    sf::Texture temp, temp2;
    
    if (firstPass || !optimized) {
      this->drawLastActivity(renderer);

      renderer.display(); // flip and ready the buffer
      last = temp = sf::Texture(renderer.getTexture()); // Make a copy of the source texture
    }
    else {
      temp = last;
    }

    sf::Sprite left(temp); 
    game::setOrigin(left, 0.5f, 0.5f);
    left.setPosition(windowSize.x/2.0f, windowSize.y/2.0f);
    left.setScale((float)alpha, (float)alpha);

    renderer.clear();

    if (firstPass || !optimized) {
      this->drawNextActivity(renderer);

      renderer.display(); // flip and ready the buffer
      next = temp2 = sf::Texture(renderer.getTexture());
    }
    else {
      temp2 = next;
    }

    sf::Sprite right(temp2);

    renderer.submit(right);
    renderer.submit(left);

    firstPass = false;
  }

  ZoomOut(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */ 
    windowSize = getController().getVirtualWindowSize();
  }

  ~ZoomOut() { ; }
};
