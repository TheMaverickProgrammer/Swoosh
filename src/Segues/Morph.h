#pragma once
#include <Swoosh\EmbedGLSL.h>
#include <Swoosh\Segue.h>
#include <Swoosh\Ease.h>

using namespace swoosh;

namespace {
  auto MORPH_SHADER = GLSL(
    110,
    uniform sampler2D texture;
    uniform sampler2D texture2;
    uniform float strength;
    uniform float time;

    void main() {
      vec2 pos = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);

      vec4 ca = texture2D(texture , pos.xy);
      vec4 cb = texture2D(texture2, pos.xy);

      vec2 oa = (((ca.rg + ca.b)*0.5)*2.0 - 1.0);
      vec2 ob = (((cb.rg + cb.b)*0.5)*2.0 - 1.0);
      vec2 oc = mix(oa, ob, 0.5)*strength;

      float w0 = time;
      float w1 = 1.0 - w0;

      gl_FragColor = mix(texture2D(texture, pos + oc * w0), texture2D(texture2, pos - oc * w1), time);
    }
  );
};

class Morph : public Segue {
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

    shader.setUniform("strength", 0.1f);
    shader.setUniform("texture2", *temp);
    shader.setUniform("texture", *temp2);
    shader.setUniform("time", (float)alpha);

    sf::RenderStates states;
    states.shader = &shader;

    surface.draw(sprite, states);

    delete temp2;
  }

  Morph(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    temp = nullptr;
    shader.loadFromMemory(::MORPH_SHADER, sf::Shader::Fragment);
  }

  virtual ~Morph() { if (temp) { delete temp; } }
};
