#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Shaders.h>

using namespace swoosh;

/**
 @class RetroBlitCustom
 @param krows. Compile-time constant of the kernel size vertically. Higher row count = bigger cells and better performance but lower quality.
 @param kcols. Compile-time constant of the kernel size horizontally. Higher col count = "" 
 
 krows and kcols determine kernel size that each is interpolated over. 
 It can be thought of as color tolerance
 For a dithering and dissolving effect, using high krows and kcols
 For a retro and blocky effect, use less
*/
template<int krows, int kcols>
class RetroBlitCustom : public Segue {
private:
  glsl::RetroBlit shader;
  sf::Texture last, next;
  bool firstPass{ true };
  bool secondPass{ true };
public:
  virtual void onDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;
    const bool useShader = getController().isShadersEnabled();

    sf::Texture temp;

    if (alpha <= 0.5) {
      if (firstPass || !optimized) {
        this->drawLastActivity(surface);

        surface.display(); // flip and ready the buffer

        last = temp = sf::Texture(surface.getTexture()); // Make a copy of the source texture
        surface.clear(this->getLastActivityBGColor());
      }
      else {
        temp = last;
      }

      shader.setTexture(&temp);
      shader.setAlpha((0.5f - (float)alpha)/0.5f);

      if(useShader) {
        shader.apply(surface);
      }

      firstPass = false;
    }
    else {
      if (secondPass || !optimized) {
        this->drawNextActivity(surface);

        surface.display(); // flip and ready the buffer

        next = temp = sf::Texture(surface.getTexture()); // Make a copy of the source texture
        sf::Sprite sprite(temp);

        surface.clear(this->getNextActivityBGColor());
      }
      else {
        temp = next;
      }

      shader.setTexture(&temp);
      shader.setAlpha(((float)alpha - 0.5f) / 0.5f);

      if(useShader) {
        shader.apply(surface);
      }

      secondPass = false;
    }
  }

  RetroBlitCustom(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next),
    shader(kcols, krows) {
    /* ... */;
  }

  ~RetroBlitCustom() { ; }
};


//!< Shorthand to use configured RetroBlit with kcols and krows of 10
using RetroBlit = RetroBlitCustom<10, 10>;
