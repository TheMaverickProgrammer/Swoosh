#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>

using namespace swoosh;

/**
  @class BlendFadeIn
  @brief Traditional screen transition effect fades out the current screen while fading in the next

  On optimized quality modes, the screen buffers will be captured only once to boost performance.
*/
class BlendFadeIn : public Segue {
private:
  int direction = 0;
  sf::Texture last, next;
  bool firstPass{ true };
public:
  void onDraw(sf::RenderTexture& surface) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;
    sf::Texture temp, temp2;

    if (firstPass || !optimized) {
      surface.clear(this->getLastActivityBGColor());
      this->drawLastActivity(surface);

      surface.display(); // flip and ready the buffer

      last = temp = sf::Texture(surface.getTexture()); // Make a copy of the source texture
    }
    else {
      temp = last;
    }

    sf::Sprite left(temp);

    if (firstPass || !optimized) {
      surface.clear(this->getNextActivityBGColor());
      this->drawNextActivity(surface);

      surface.display(); // flip and ready the buffer
      next = temp2 = sf::Texture(surface.getTexture());
    }
    else {
      temp2 = next;
    }
   
    sf::Sprite right(temp2);

    left.setColor(sf::Color(255, 255, 255, (sf::Uint8)((1.0-alpha) * 255.0)));
    right.setColor(sf::Color(255, 255, 255, (sf::Uint8)(alpha * 255.0)));

    surface.draw(left);
    surface.draw(right);

    firstPass = false;
  }

  BlendFadeIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
  }

  ~BlendFadeIn() { ; }
};
