#pragma once
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/Shaders.h>

using namespace swoosh;

/**
  @class CircleOpen
  @brief Reveals the next scene behind a shrinking circle

  If optimized for mobile, will capture the scenes once and use less vertices to increase performance on weak hardware
*/
class CircleClose : public Segue {
private:
  glsl::CircleMask shader;
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

    sf::Vector2u size = getController().getWindow().getSize();
    float aspectRatio = (float)size.x / (float)size.y;

    shader.setAlpha(1.0f-(float)alpha);
    shader.setAspectRatio(aspectRatio);
    shader.setTexture(&temp);

    if(useShader) {
      shader.apply(renderer);
    }

    renderer.display();
    sf::Texture temp3(renderer.getTexture());

    sf::Sprite left(temp2);
    sf::Sprite right(temp3);

    renderer.submit(left);
    renderer.submit(right);

    firstPass = false;
  }

  CircleClose(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
  }

  ~CircleClose() { }
};
