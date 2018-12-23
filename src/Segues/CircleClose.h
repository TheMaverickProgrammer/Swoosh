#pragma once
#include <Swoosh\EmbedGLSL.h>
#include <Swoosh\Segue.h>
#include <Swoosh\Ease.h>

using namespace swoosh;

namespace {
  auto CIRCLE_CLOSE_SHADER = GLSL(
    110,
    uniform sampler2D texture;
    uniform sampler2D texture2;
    uniform float ratio;
    uniform float time;

    void main() {
      vec2 pos = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);

      float size = 1.0-time;

      vec4 outcol = texture2D(texture2, gl_TexCoord[0].xy);

      float x = pos.x - 0.5;
      float y = pos.y - 0.5;

      if (ratio >= 1.0) {
        x *= ratio;
      }
      else {
        y *= 1.0/ratio;
      }

      if (x*x + y*y < size*size) {
        outcol = texture2D(texture, gl_TexCoord[0].xy);
      }

      gl_FragColor = outcol;
    }
  );
};

class CircleClose : public Segue {
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

    sf::Vector2u size = getController().getWindow().getSize();
    float aspectRatio = (float)size.x / (float)size.y;

    shader.setUniform("ratio", aspectRatio);
    shader.setUniform("texture2", *temp);
    shader.setUniform("texture", *temp2);
    shader.setUniform("time", (float)alpha);

    sf::RenderStates states;
    states.shader = &shader;

    surface.draw(sprite, states);

    delete temp2;
  }

  CircleClose(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    temp = nullptr;
    shader.loadFromMemory(::CIRCLE_CLOSE_SHADER, sf::Shader::Fragment);
  }

  virtual ~CircleClose() { if (temp) { delete temp; } }
};
