#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Game.h>
#include <Swoosh/Ease.h>

using namespace swoosh;

/**
  @class ZoomIn
  @brief brings the next sceen in from the center of the screen by scaling up quickly

  If requested quality is set to mobile, capture the first screen and do not capture real-time.
 */

class ZoomIn : public Segue {
private:
  sf::Vector2u windowSize;
  sf::Texture next, last;
  bool firstPass{ true };

public:
 void onDraw(IRenderer& renderer) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::bezierPopIn(elapsed, duration);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;

    sf::Texture temp, temp2;

    if (firstPass || !optimized) {
      this->drawNextActivity(renderer);

      renderer.display(); // flip and ready the buffer
      next = temp = sf::Texture(renderer.getTexture()); // Make a copy of the source texture
    }
    else {
      temp = next;
    }

    sf::Sprite left(temp); 
    game::setOrigin(left, 0.5f, 0.5f);
    left.setPosition((float)(windowSize.x/2.0f), (float)(windowSize.y/2.0f));
    left.setScale((float)alpha, (float)alpha);

    renderer.clear();

    if (firstPass || !optimized) {
      this->drawLastActivity(renderer);

      renderer.display(); // flip and ready the buffer
      last = temp2 = sf::Texture(renderer.getTexture());
    }
    else {
      temp2 = last;
    }

    sf::Sprite right(temp2);

    renderer.submit(right);
    renderer.submit(left);

    firstPass = false;
  }

  ZoomIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */ 
    windowSize = getController().getVirtualWindowSize();
  }

  ~ZoomIn() { }
};
