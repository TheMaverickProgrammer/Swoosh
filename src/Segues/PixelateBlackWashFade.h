#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/EmbedGLSL.h>

using namespace swoosh;

namespace {
  auto PIXELATE_SHADER = GLSL
  (
    110,
    uniform sampler2D texture;
    uniform float pixel_threshold;

    void main()
    {
      float factor = 1.0 / (pixel_threshold + 0.001);
      vec2 pos = floor(gl_TexCoord[0].xy * factor + 0.5) / factor;
      gl_FragColor = texture2D(texture, pos) * gl_Color;
    }
  );
}

class PixelateBlackWashFade : public Segue {
private:
  sf::Shader shader;
  float factor;

public:
  virtual void onDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::wideParabola(elapsed, duration, 1.0);

    if (elapsed <= duration * 0.5)
      this->drawLastActivity(surface);
    else
      this->drawNextActivity(surface);

    surface.display();
    sf::Texture* temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture
    sf::Sprite sprite(*temp);

    shader.setUniform("texture", *temp);
    shader.setUniform("pixel_threshold", (float)alpha/15.0f);
    sf::RenderStates states;
    states.shader = &shader;

    surface.clear(sf::Color::Transparent);
    surface.draw(sprite, states);

    // 10% of segue is a pixelate before darkening
    double delay = (duration / 10.0);
    if (elapsed > delay) {
      double alpha = ease::wideParabola(elapsed - delay, duration - delay, 1.0);

      sf::RectangleShape whiteout;
      whiteout.setSize(sf::Vector2f((float)surface.getTexture().getSize().x, (float)surface.getTexture().getSize().y));
      whiteout.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(alpha * (double)255)));
      surface.draw(whiteout);
    }
  }

  PixelateBlackWashFade(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    shader.loadFromMemory(::PIXELATE_SHADER, sf::Shader::Fragment);
  }

  virtual ~PixelateBlackWashFade() { ; }
};
