#pragma once
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/Shaders.h>

using namespace swoosh;

/**
  @class RadialCCW
  @brief The next screen is revealed with a clock-wise swipe effect over time

  In mobile, the screen contents are captured once and not real-time
*/

class RadialCCW : public Segue {
private:
  glsl::RadialCCW shader;
  sf::Texture next, last;
  bool firstPass{ true };
public:
 void onDraw(IRenderer& renderer) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;
    const bool useShader = getController().isShadersEnabled();

    sf::Texture temp, temp2;

    renderer.clear(this->getLastActivityBGColor());

    if (firstPass || !optimized) {
      this->drawLastActivity(renderer);

      renderer.display(); // flip and ready the buffer

      last = temp = sf::Texture(renderer.getTexture()); // Make a copy of the source texture
    }
    else {
      temp = last;
    }

    renderer.clear(this->getNextActivityBGColor());

    if (firstPass || !optimized) {
      this->drawNextActivity(renderer);

      renderer.display(); // flip and ready the buffer
      next = temp2 = sf::Texture(renderer.getTexture()); // Make a copy of the source texture
    }
    else {
      temp2 = next;
    }

    shader.setTexture1(&temp);
    shader.setTexture2(&temp2);
    shader.setAlpha((float)alpha);

    if(useShader) {
      shader.apply(renderer);
    }

    firstPass = false;
  }

  RadialCCW(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
  }

  ~RadialCCW() { }
};
