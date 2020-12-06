#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>

using namespace swoosh;

/**
  @class PushIn
  @brief Pushes the new screen in while pushing the last screen out
  @param direction. Compile-time constant. A cardinal direction to push from.
*/
template<types::direction direction>
class PushIn : public Segue {
  sf::Texture next, last;
  bool firstPass{ true };
public:

 void onDraw(sf::RenderTexture& surface) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);
    bool optimized = getController().getRequestedQuality() == quality::mobile;

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

    int lr = 0;
    int ud = 0;

    if (direction == types::direction::left ) lr = -1;
    if (direction == types::direction::right) lr = 1;
    if (direction == types::direction::up   ) ud = -1;
    if (direction == types::direction::down ) ud = 1;

    left.setPosition((float)(lr * alpha * left.getTexture()->getSize().x), (float)(ud * alpha * left.getTexture()->getSize().y));

    surface.clear(this->getNextActivityBGColor());

    if (firstPass || !optimized) {
      this->drawNextActivity(surface);

      surface.display(); // flip and ready the buffer

      next = temp2 = sf::Texture(surface.getTexture());
    }
    else {
      temp2 = next;
    }

    sf::Sprite right(temp2);

    right.setPosition((float)(-lr * (1.0-alpha) * right.getTexture()->getSize().x), (float)(-ud * (1.0-alpha) * right.getTexture()->getSize().y));

    surface.draw(left);
    surface.draw(right);

    firstPass = false;
  }

  PushIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
  }

  ~PushIn() { }
};
