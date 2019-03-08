#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/EmbedGLSL.h>

using namespace swoosh;

namespace {
  auto CHECKERBOARD_FRAG_SHADER = GLSL
  (
    110,
    uniform sampler2D texture;
    uniform sampler2D texture2;
    uniform float progress;
    uniform float smoothness; // = 0.5
    uniform int cols;
    uniform int rows;

    float rand(vec2 co) {
      return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
    }

    void main() {
      vec2 p = gl_TexCoord[0].xy;
      vec2 size = vec2(cols, rows);
      float r = rand(floor(vec2(size) * p));
      float m = smoothstep(0.0, -smoothness, r - (progress * (1.0 + smoothness)));
      gl_FragColor = mix(texture2D(texture, p.xy), texture2D(texture2, p.xy), m);
    }

   );
}

template<int cols, int rows>
class CheckerboardCustom : public Segue {
private:
  sf::Texture* temp;
  sf::Shader shader;

public:
  virtual void onDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);

    this->drawLastActivity(surface);

    surface.display(); // flip and ready the buffer

    if (temp) delete temp;
    temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    sf::Sprite sprite(*temp);

    surface.clear(sf::Color::Transparent);
    this->drawNextActivity(surface);

    surface.display(); // flip and ready the buffer

    sf::Texture* temp2 = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    shader.setUniform("progress", (float)alpha);
    shader.setUniform("texture2", *temp2);
    shader.setUniform("texture", *temp);

    sf::RenderStates states;
    states.shader = &shader;

    surface.draw(sprite, states);

    delete temp2;
  }

  CheckerboardCustom(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    temp = nullptr;

    shader.loadFromMemory(::CHECKERBOARD_FRAG_SHADER, sf::Shader::Fragment);
    shader.setUniform("cols", cols);
    shader.setUniform("rows", rows);
    shader.setUniform("smoothness", 0.09f);
  }

  virtual ~CheckerboardCustom() { ; }
};

using Checkerboard = CheckerboardCustom<10, 10>;
