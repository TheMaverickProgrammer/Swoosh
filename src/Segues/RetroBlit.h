#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Shaders.h>

using namespace swoosh;

/**
 @class RetroBlit
 Reduces the color channel bit rate over time to give the impression of dithering out like some older games
*/
class RetroBlit: public Segue {
private:
  glsl::RetroBlit shader;
  sf::Texture last, next;
  bool firstPass{ true };
  bool secondPass{ true };
public:
  virtual void onDraw(IRenderer& renderer) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;
    const bool useShader = getController().isShadersEnabled();

    sf::Texture temp;

    if (alpha <= 0.5) {
      if (firstPass || !optimized) {
        this->drawLastActivity(renderer);

        renderer.display(); // flip and ready the buffer

        last = temp = sf::Texture(renderer.getTexture()); // Make a copy of the source texture
        renderer.clear(this->getLastActivityBGColor());
      }
      else {
        temp = last;
      }

      shader.setTexture(&temp);
      shader.setAlpha((0.5f - (float)alpha)/0.5f);

      if(useShader) {
        shader.apply(renderer);
      }

      firstPass = false;
    }
    else {
      if (secondPass || !optimized) {
        this->drawNextActivity(renderer);

        renderer.display(); // flip and ready the buffer

        next = temp = sf::Texture(renderer.getTexture()); // Make a copy of the source texture
        sf::Sprite sprite(temp);

        renderer.clear(this->getNextActivityBGColor());
      }
      else {
        temp = next;
      }

      shader.setTexture(&temp);
      shader.setAlpha(((float)alpha - 0.5f) / 0.5f);

      if(useShader) {
        shader.apply(renderer);
      }

      secondPass = false;
    }
  }

  RetroBlit(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next),
    shader() {
    /* ... */;
  }

  ~RetroBlit() { ; }
};