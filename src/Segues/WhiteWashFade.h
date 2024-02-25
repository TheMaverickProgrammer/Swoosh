#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>

using namespace swoosh;

/**
  @class WhiteWashFade
  @brief The last scene fades out white and the new scene fades in

  Since this is a very simple effect, all quality modes are the same
 */

class WhiteWashFade : public Segue {
public:
 void onDraw(IRenderer& renderer) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::wideParabola(elapsed, duration, 1.0);

    if (elapsed <= duration * 0.5)
      this->drawLastActivity(renderer);
    else
      this->drawNextActivity(renderer);

    sf::RectangleShape whiteout;
    whiteout.setSize(sf::Vector2f((float)renderer.getTexture().getSize().x, (float)renderer.getTexture().getSize().y));
    whiteout.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)(alpha*255)));
    renderer.submit(Immediate(&whiteout));
  }

  WhiteWashFade(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) { /* ... */ }
  ~WhiteWashFade() { ; }
};
