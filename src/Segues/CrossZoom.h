#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Shaders.h>

using namespace swoosh;

/**
  @class CrossZoomCustom
  @brief Fades the next sceen in by projecting the scene outward with a blinding-light effect
  @warning CPU hog on mobile. Even when optimizing SFML bottlenecks on mobile hardware with this shader.
  @param percent_power. Compile-time constant can be values 0-100 of effect intensity
 */

template<int percent_power> // from 0% - 100% 
class CrossZoomCustom : public Segue {
private:
  sf::Texture next, last;
  glsl::CrossZoom shader;
  bool firstPass{ true };
public:
 void onDraw(sf::RenderTexture& surface) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);
    const bool optimized = getController().isOptimizedForPerformance();
    const bool useShader = getController().isShadersEnabled();

    sf::Texture temp, temp2;

    surface.clear(this->getLastActivityBGColor());

    if (firstPass || !optimized) {
      this->drawLastActivity(surface);

      surface.display(); // flip and ready the buffer
      last = temp = sf::Texture(surface.getTexture()); // Make a copy of the source texture
    }
    else {
      temp = last;
    }

    sf::Sprite sprite(temp);

    surface.clear(this->getNextActivityBGColor());

    if (firstPass || !optimized) {
      this->drawNextActivity(surface);

      surface.display(); // flip and ready the buffer

      next = temp2 = sf::Texture(surface.getTexture());
    }
    else {
      temp2 = next;
    }
  
    shader.setAlpha((float)alpha);
    shader.setPower((float)percent_power / 100.0f);
    shader.setTexture1(&temp);
    shader.setTexture2(&temp2);

    if(useShader) {
      shader.apply(surface);
    }

    firstPass = false;
  }

  CrossZoomCustom(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
  }

  ~CrossZoomCustom() { ; }
};

//!< Shorthand for a cross zoom effect with 40% power
using CrossZoom = CrossZoomCustom<40>;
