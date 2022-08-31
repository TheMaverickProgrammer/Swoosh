#pragma once
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/Shaders.h>

using namespace swoosh;

/**
  @class Moprh
  @brief Transforms the last scene into the next

  If optimized for mobile, will capture the scenes once to increase performance on weak hardware
*/
class Morph : public Segue {
private:
  glsl::Morph shader;
  sf::Texture last, next;
  bool firstPass{ true };

public:
 void onDraw(IRenderer& renderer) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;
    const bool useShader = getController().isShadersEnabled();

    sf::Texture temp, temp2;

    if (firstPass || !optimized) {
      renderer.clear(this->getLastActivityBGColor());
      this->drawLastActivity(renderer);

      renderer.display(); // flip and ready the buffer
      last = temp = sf::Texture(renderer.getTexture()); // Make a copy of the source texture
    }
    else {
      temp = last;
    }

    if (firstPass || !optimized) {
      renderer.clear(this->getNextActivityBGColor());
      this->drawNextActivity(renderer);

      renderer.display(); // flip and ready the buffer
      next = temp2 = sf::Texture(renderer.getTexture()); // Make a copy of the source texture
    }
    else {
      temp2 = next;
    }

    shader.setAlpha((float)alpha);
    shader.setTexture1(&temp);
    shader.setTexture2(&temp2);

    if(useShader) {
      shader.apply(renderer);
    }

    firstPass = false;
  }

  Morph(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    shader.setStrength(0.1f);
  }

  ~Morph() { }
};
