#pragma once
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/Shaders.h>

using namespace swoosh;

/**
  @class CircleOpen
  @brief Reveals the next scene inside an expanding circle, eventually replacing the last scene completely

  If optimized for mobile, will capture the scenes once and use less vertices to increase performance on weak hardware
*/
class CircleOpen : public Segue {
private:
  glsl::CircleMask shader;
  sf::Texture next, last;
  bool firstPass{ true };
public:
 void onDraw(sf::RenderTexture& surface) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;
    const bool useShader = getController().isShadersEnabled();

    sf::Texture temp, temp2;

    if (firstPass || !optimized) {
      surface.clear(this->getNextActivityBGColor());
      this->drawNextActivity(surface);
      surface.display(); // flip and ready the buffer

      next = temp = sf::Texture(surface.getTexture()); // Make a copy of the source texture
    }
    else {
      temp = next;
    }

    if (firstPass || !optimized) {
      surface.clear(this->getLastActivityBGColor());
      this->drawLastActivity(surface);
      surface.display(); // flip and ready the buffer

      last = temp2 = sf::Texture(surface.getTexture()); // Make a copy of the source texture
    }
    else {
      temp2 = last;
    }

    sf::Vector2u size = getController().getWindow().getSize();
    float aspectRatio = (float)size.x / (float)size.y;

    shader.setAlpha((float)alpha);
    shader.setAspectRatio(aspectRatio);
    shader.setTexture(&temp);

    if(useShader) {
      shader.apply(surface);
    }

    surface.display();
    sf::Texture temp3(surface.getTexture());
    sf::Sprite left(temp3);
    sf::Sprite right(temp2);

    surface.draw(right);
    surface.draw(left);

    firstPass = false;
  }

  CircleOpen(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
  }

  ~CircleOpen() { }
};
