#pragma once
#include <Swoosh\EmbedGLSL.h>
#include <Swoosh\Segue.h>
#include <Swoosh\Ease.h>

using namespace swoosh;

namespace {
  auto RADIAL_CCW_SHADER = GLSL(
    110,
    uniform sampler2D texture;
    uniform sampler2D texture2;
    uniform float time;

    void main() {
      vec2 pos = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);

      vec4 from = texture2D(texture , pos.xy);
      vec4 to   = texture2D(texture2, pos.xy);

      const float PI = 3.141592653589;

        vec2 rp = pos * 2.0 - 1.0;
        gl_FragColor = mix(
          texture2D(texture,  pos),
          texture2D(texture2, pos),
          smoothstep(0.0, 0.0, atan(rp.y, rp.x) - (1.0-time - .5) * PI * 2.5)
        );
    }
  );
};

class RadialCCW : public Segue {
private:
  sf::Texture* temp;
  sf::Shader shader;

public:
  virtual void onDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);

    this->drawNextActivity(surface);

    surface.display(); // flip and ready the buffer

    if (temp) delete temp;
    temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    sf::Sprite sprite(*temp);

    surface.clear(sf::Color::Transparent);
    this->drawLastActivity(surface);

    surface.display(); // flip and ready the buffer

    sf::Texture* temp2 = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    shader.setUniform("texture", *temp2);
    shader.setUniform("texture2", *temp);
    shader.setUniform("time", (float)alpha);

    sf::RenderStates states;
    states.shader = &shader;

    surface.draw(sprite, states);

    delete temp2;
  }

  RadialCCW(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    temp = nullptr;
    shader.loadFromMemory(::RADIAL_CCW_SHADER, sf::Shader::Fragment);
  }

  virtual ~RadialCCW() { if (temp) { delete temp; } }
};
