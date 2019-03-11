#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/EmbedGLSL.h>

using namespace swoosh;

/*
Custom stepping blit shader by TheMaverickProgrammer
*/
auto RETRO_BLIT_SHADER = GLSL(
  110,
  uniform sampler2D texture;
  uniform float progress;
  uniform int cols;
  uniform int rows;

  float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
  }

  void main() {
    vec2 p = gl_TexCoord[0].xy;
    vec2 size = vec2(cols, rows);
    vec4 color = texture2D(texture, p.xy);
    float r = rand(floor(vec2(size) * color.xy));
    float m = smoothstep(0.0, 0.0, r - (progress * (1.0)));
    gl_FragColor = mix(color, vec4(0.0, 0.0, 0.0, 1.0), m);
  }
);

// krows and kcols determine kernel size that each is interpolated over. 
// For a dissolving effect, using high krows and kcols
// For a retro effect, use less
template<int krows, int kcols>
class RetroBlitCustom : public Segue {
private:
  sf::Shader shader;

public:
  virtual void onDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);

    sf::Texture* temp = nullptr;

    if (alpha <= 0.5) {
      this->drawLastActivity(surface);

      surface.display(); // flip and ready the buffer

      temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

      sf::Sprite sprite(*temp);

      surface.clear(sf::Color::Transparent);

      shader.setUniform("texture", *temp);
      shader.setUniform("progress", (0.5f - (float)alpha)/0.5f);

      sf::RenderStates states;
      states.shader = &shader;

      surface.draw(sprite, states);
    }
    else {

      this->drawNextActivity(surface);

      surface.display(); // flip and ready the buffer

      sf::Texture* temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

      sf::Sprite sprite(*temp);

      shader.setUniform("texture", *temp);
      shader.setUniform("progress", ((float)alpha - 0.5f)/0.5f);

      sf::RenderStates states;
      states.shader = &shader;

      surface.draw(sprite, states);
    }

    delete temp;
  }

  RetroBlitCustom(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */;

    shader.loadFromMemory(::RETRO_BLIT_SHADER, sf::Shader::Fragment);
    shader.setUniform("cols", kcols);
    shader.setUniform("rows", krows);
  }

  virtual ~RetroBlitCustom() { ; }
};

using RetroBlit = RetroBlitCustom<10, 10>;
