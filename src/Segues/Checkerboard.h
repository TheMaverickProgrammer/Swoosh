#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/EmbedGLSL.h>

using namespace swoosh;

/**
  @class CheckerboardCustom
  @brief A grid blocks-out the next scene over time
  @param cols Compile-time constant. number of cols to divide the grid into.
  @param rows Compile-time constant. number of rows to divide the grid into.

  If optimized for mobile, will capture the scenes once
*/
template<int cols, int rows>
class CheckerboardCustom : public Segue {
private:
  sf::Texture next, last;
  sf::Shader shader;
  bool firstPass{ true };
  std::string checkerboardShader;
public:
  void onDraw(sf::RenderTexture& surface) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;
    const bool useShader = getController().isShadersEnabled();

    sf::Texture temp, temp2;

    if (firstPass || !optimized) {
      drawLastActivity(surface);

      surface.display(); // flip and ready the buffer

#ifdef __ANDROID__
      temp = sf::Texture(surface.getTexture()); // Make a copy of the source texture
      temp.flip(true);
#else
      last = temp = sf::Texture(surface.getTexture()); // Make a copy of the source texture
#endif
    }
    else {
      temp = last;
    }
    sf::Sprite sprite(temp);

    if (firstPass || !optimized) {
      surface.clear(sf::Color::Transparent);
      drawNextActivity(surface);

      surface.display(); // flip and ready the buffer

#ifdef __ANDROID__
      temp2 = sf::Texture(surface.getTexture()); // Make a copy of the source texture
      temp2.flip(true);
#else
      next = temp2 = sf::Texture(surface.getTexture()); // Make a copy of the source texture
#endif
    }
    else {
      temp2 = next;
    }

    shader.setUniform("progress", (float)alpha);
    shader.setUniform("texture2", temp2);
    shader.setUniform("texture", temp);

    sf::RenderStates states;

    if(useShader) {
      states.shader = &shader;
    }

    surface.draw(sprite, states);
  }

  CheckerboardCustom(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
#ifdef __ANDROID__
      this->checkerboardShader = GLSL(
        100,
        precision highp float;
        precision highp int;
        precision highp sampler2D;

        varying vec2 vTexCoord;
        varying vec4 vColor;

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
          vec2 p = vTexCoord.xy;

          vec2 size = vec2(cols, rows);
          float r = rand(floor(vec2(size) * p));
          float m = smoothstep(0.0, -smoothness, r - (progress * (1.0 + smoothness)));
          gl_FragColor = mix(texture2D(texture, p.xy), texture2D(texture2, p.xy), m);
       }
     );
#else
      this->checkerboardShader = GLSL
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
#endif

    shader.loadFromMemory(checkerboardShader, sf::Shader::Fragment);

    shader.setUniform("cols", cols);
    shader.setUniform("rows", rows);
    shader.setUniform("smoothness", 0.09f);
  }

  ~CheckerboardCustom() {
  }
};

//!< Shorthand for configured checkerboard of 10x10 grid
using Checkerboard = CheckerboardCustom<10, 10>;
