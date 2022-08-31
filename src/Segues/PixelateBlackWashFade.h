#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Shaders.h>

using namespace swoosh;

/**
  @class PixelateBlackWashFade
  @brief Pixelates while fading black as seen in the original Super Mario game for SNES

  If optimized for mobile, will capture the scenes once and use less vertices to increase performance on weak hardware
*/
class PixelateBlackWashFade : public Segue {
private:
  glsl::Pixelate shader;
  sf::Texture last, next;
  bool firstPass{ true };
  bool secondPass{ true };
public:
 void onDraw(IRenderer& renderer) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::wideParabola(elapsed, duration, 1.0);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;
    const bool useShader = getController().isShadersEnabled();

    sf::Texture temp;

    if (elapsed <= duration * 0.5) {
      if (firstPass || !optimized) {
        renderer.clear(this->getLastActivityBGColor());
        this->drawLastActivity(renderer);
        renderer.display();
        last = temp = sf::Texture(renderer.getTexture()); // Make a copy of the source texture

        firstPass = false;
      }
      else {
        temp = last;
      }
    }
    else {
      if (secondPass || !optimized) {
        renderer.clear(this->getNextActivityBGColor());
        this->drawNextActivity(renderer);
        renderer.display();
        next = temp = sf::Texture(renderer.getTexture()); // Make a copy of the source texture

        secondPass = false;
      }
      else {
        temp = next;
      }
    }


    shader.setTexture(&temp);
    shader.setThreshold((float)alpha/15.0f);

    if(useShader) {
      shader.apply(renderer);
    }

    // 10% of segue is a pixelate before darkening
    double delay = (duration / 10.0);
    if (elapsed > delay) {
      double alpha = ease::wideParabola(elapsed - delay, duration - delay, 1.0);

      sf::RectangleShape blackout;
      blackout.setSize(sf::Vector2f((float)renderer.getTexture().getSize().x, (float)renderer.getTexture().getSize().y));
      blackout.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(alpha * (double)255)));
      renderer.submit(blackout);
    }
  }

  PixelateBlackWashFade(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
  }

  ~PixelateBlackWashFade() { ; }
};
