#pragma once
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>

using namespace swoosh;

namespace {
  auto DIAMOND_SHADER = GLSL(
    110,
    uniform sampler2D texture;
    uniform float time;
    uniform int direction;

    float modulo(float a, float b) {
      return a - (b * floor(a / b));
    }

    void main() {
      // Map range time from (0.0, 1.0) to (0.5,2.25) for the equation to cover the screen
      float range = 0.50*(1.0 - time) + (2.25*time);

      //Scale the uvs to integers to scale directly with the equation.
      vec2 posI = vec2(gl_TexCoord[0].x * 40.0, gl_TexCoord[0].y * 30.0);
      //modulo the position to clamp it to repeat the pattern.
      vec2 pos = vec2(modulo(posI.x, 2.0), modulo(posI.y, 2.0)) - vec2(1.0, 1.0);
      float size;

      if(direction == 0) {
          size = pow(range - (1.0 - gl_TexCoord[0].x), 3.0);
      } else if(direction == 1) {
          size = pow(range - gl_TexCoord[0].x, 3.0);
      } else if(direction == 2) {
          size = pow(range - (1.0 - gl_TexCoord[0].y), 3.0);
      } else if(direction == 3) {
          size = pow(range - gl_TexCoord[0].y, 3.0);
      } else {
          size = pow(range - (1.0 - gl_TexCoord[0].x), 3.0);
      }

      size = abs(size);
      vec4 outcol = texture2D(texture, gl_TexCoord[0].xy);

      if (abs(pos.x) + abs(pos.y) < size) {
        outcol = vec4(0, 0, 0, 1);
      }

      gl_FragColor = outcol;
    }
  );
};

template<int direction>
class DiamondTileSwipe : public Segue {
private:
  sf::Texture* temp;
  sf::Shader shader;

public:
  virtual void onDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::wideParabola(elapsed, duration, 1.0);

    if (elapsed < duration * 0.5)
      this->drawLastActivity(surface);
    else
      this->drawNextActivity(surface);

    surface.display(); // flip and ready the buffer

    if (temp) delete temp;
    temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    sf::Sprite sprite(*temp);

    shader.setUniform("texture", *temp);
    shader.setUniform("direction", direction);
    shader.setUniform("time", (float)alpha);

    sf::RenderStates states;
    states.shader = &shader;

    surface.draw(sprite, states);
  }

  DiamondTileSwipe(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    temp = nullptr;
    shader.loadFromMemory(::DIAMOND_SHADER, sf::Shader::Fragment);
  }

  virtual ~DiamondTileSwipe() { if (temp) { delete temp; } }
};
