#pragma once
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/Shaders.h>

using namespace swoosh;

/**
  @class BlurFadeIn
  @brief Blurs both screens and fades into the next while the last fades out
  @warning COSTLY! For mobile, blurring will be turned off and it will be a simple fade
*/
class BlurFadeIn : public Segue {
private:
  glsl::FastGaussianBlur shader;
  sf::Texture last, next;
  bool firstPass{ true };

  const int kernels(const quality& mode) {
    switch (mode) {
    case quality::realtime:
      return 60;
    case quality::reduced:
      return 30;
    }

    // quality::mobile
    return 10;
  }

public:
  void onDraw(IRenderer& renderer) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::wideParabola(elapsed, duration, 1.0);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;
    const bool useShader = getController().isShadersEnabled();

    shader.setPower((float)alpha * 8.f);

    renderer.clear(this->getLastActivityBGColor());
    sf::Texture temp, temp2;

    if (firstPass || !optimized) {
      this->drawLastActivity(renderer);

      renderer.display(); // flip and ready the buffer
      last = temp = sf::Texture(renderer.getTexture()); // Make a copy of the source texture
    }
    else {
      temp = last;
    }

    shader.setTexture(&temp);

    if(useShader) {
      shader.apply(renderer);
    }

    renderer.display();
    temp = sf::Texture(renderer.getTexture());

    renderer.clear(this->getNextActivityBGColor());

    if (firstPass || !optimized) {
      this->drawNextActivity(renderer);

      renderer.display(); // flip and ready the buffer
      next = temp2 = sf::Texture(renderer.getTexture()); // Make a copy of the source texture
    }
    else {
      temp2 = next;
    }

    shader.setTexture(&temp2);

    if(useShader) {
      shader.apply(renderer);
    }

    renderer.display();
    temp2 = sf::Texture(renderer.getTexture());

    sf::Sprite sprite, sprite2;
    sprite.setTexture(temp);
    sprite2.setTexture(temp2);

    renderer.clear(sf::Color::Transparent);
    alpha = ease::linear(elapsed, duration, 1.0);

    sprite.setColor(sf::Color(255, 255, 255, (sf::Uint8)(255.0 * (1-alpha))));
    sprite2.setColor(sf::Color(255, 255, 255, (sf::Uint8)(255.0 * alpha)));

    renderer.submit(sprite);
    renderer.submit(sprite2);

    firstPass = false;
  }

  BlurFadeIn(sf::Time duration, Activity* last, Activity* next) 
    // different kernels for each quality mode
    : Segue(duration, last, next), shader(kernels(next->getController().getRequestedQuality())) {
    /*...*/
  }

  ~BlurFadeIn() { ; }
};
