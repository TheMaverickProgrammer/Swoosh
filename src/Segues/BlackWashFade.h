#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>

using namespace swoosh;

/**
  @class BlendFadeIn
  @brief Traditional screen transition effect fades out the current screen while fading in the next

  The effect is the same across all optimized modes
*/
class BlackWashFade : public Segue {
public:
  void onDraw(IRenderer& renderer) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::wideParabola(elapsed, duration, 1.0);

    if (elapsed <= duration * 0.5) {
      renderer.clear(this->getLastActivityBGColor());
      this->drawLastActivity(renderer);
    }
    else {
      renderer.clear(this->getNextActivityBGColor());
      this->drawNextActivity(renderer);
    }

    sf::RectangleShape whiteout;
    whiteout.setSize(sf::Vector2f((float)renderer.getTexture().getSize().x, (float)renderer.getTexture().getSize().y));
    whiteout.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(alpha*255)));
    renderer.submit(Immediate(&whiteout));
  }

  BlackWashFade(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) { /* ... */ }
  ~BlackWashFade() { ; }
};
