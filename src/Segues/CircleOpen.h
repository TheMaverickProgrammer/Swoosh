#pragma once
#include <Swoosh\EmbedGLSL.h>
#include <Swoosh\Segue.h>
#include <Swoosh\Ease.h>

using namespace swoosh;

namespace {
  auto CIRCLE_OPEN_SHADER = GLSL(
    110,
    uniform sampler2D texture;
    uniform sampler2D texture2;
    uniform float time;

    void main() {
      vec2 pos = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);

      float size = time;
      vec4 outcol = texture2D(texture2, gl_TexCoord[0].xy);

      if (pow(pos.x-0.5, 2.0)+pow(pos.y-0.5, 2.0) < size*size) {
          outcol = texture2D(texture, gl_TexCoord[0].xy);
      }

      gl_FragColor = outcol;
    }
  );
};

class CircleOpen : public Segue {
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

    shader.setUniform("texture2", *temp2);
    shader.setUniform("texture", *temp);
    shader.setUniform("time", (float)alpha);

    sf::RenderStates states;
    states.shader = &shader;

    surface.draw(sprite, states);

    delete temp2;
  }

  CircleOpen(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    temp = nullptr;
    shader.loadFromMemory(::CIRCLE_OPEN_SHADER, sf::Shader::Fragment);
  }

  virtual ~CircleOpen() { if (temp) { delete temp; } }
};
