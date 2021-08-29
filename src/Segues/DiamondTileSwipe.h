#pragma once
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>

using namespace swoosh;

/**
  @class DiamondTileSwipe
  @brief Fills the screen with black tiled diamonds and then reveals the next scene in the same way
  @param direction. Compile-time enum constant that determines which direction to swipe to

  Similar effect seen in Cave Story

  If optimized for mobile, will capture the scenes once and use less vertices to increase performance on weak hardware
*/
template<types::direction direction>
class DiamondTileSwipe : public Segue {
private:
  sf::Texture last, next;
  sf::Shader shader;
  std::string diamondSwipeShaderProgram;
  bool firstPass{ true }, secondPass{ true };
public:
 void onDraw(sf::RenderTexture& surface) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::wideParabola(elapsed, duration, 1.0);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;
    const bool useShader = getController().isShadersEnabled();

    sf::Texture temp;

    if (elapsed < duration * 0.5) {
      if (firstPass || !optimized) {
        this->drawLastActivity(surface);
        surface.display(); // flip and ready the buffer
        last = temp = sf::Texture(surface.getTexture()); // Make a copy of the source texture
        firstPass = false;
      }
      else {
        temp = last;
      }
    }
    else {
      if (secondPass || !optimized) {
        this->drawNextActivity(surface);
        surface.display(); // flip and ready the buffer
        next = temp = sf::Texture(surface.getTexture()); // Make a copy of the source texture
        firstPass = false;
      }
      else {
        temp = next;
      }
    }

    sf::Sprite sprite(temp);

    shader.setUniform("texture", temp);
    shader.setUniform("direction", static_cast<int>(direction));
    shader.setUniform("time", (float)alpha);

    sf::RenderStates states;

    if(useShader) {
      states.shader = &shader;
    }

    surface.draw(sprite, states);
  }

  DiamondTileSwipe(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    this->diamondSwipeShaderProgram = GLSL(
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

        if (direction == 0) {
          size = pow(range - (1.0 - gl_TexCoord[0].x), 3.0);
        }
        else if (direction == 1) {
          size = pow(range - gl_TexCoord[0].x, 3.0);
        }
        else if (direction == 2) {
          size = pow(range - (1.0 - gl_TexCoord[0].y), 3.0);
        }
        else if (direction == 3) {
          size = pow(range - gl_TexCoord[0].y, 3.0);
        }
        else {
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

    shader.loadFromMemory(diamondSwipeShaderProgram, sf::Shader::Fragment);
  }

  ~DiamondTileSwipe() { }
};
