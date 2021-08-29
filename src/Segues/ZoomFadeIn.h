#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/EmbedGLSL.h>

using namespace swoosh;

/**
  @class ZoomFadeIn
  @brief brings the next sceen in by scaling the last scene up and fading out as if it was approaching the camera

  If requested quality is set to mobile, capture the first screen and do not capture real-time.
*/
class ZoomFadeIn : public Segue {
private:
  std::string zoomShaderProgram;
  sf::Shader shader;
  sf::Texture next, last;
  bool firstPass{ true };
public:
 void onDraw(sf::RenderTexture& surface) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;
    const bool useShader = getController().isShadersEnabled();

    sf::Texture temp, temp2;

    if (firstPass || !optimized) {
      this->drawLastActivity(surface);

      surface.display(); // flip and ready the buffer

      last = temp = sf::Texture(surface.getTexture()); // Make a copy of the source texture
    }
    else {
      temp = last;
    }

    sf::Sprite sprite(temp);

    surface.clear(sf::Color::Transparent);

    if (firstPass || !optimized) {
      this->drawNextActivity(surface);

      surface.display(); // flip and ready the buffer

      next = temp2 = sf::Texture(surface.getTexture()); // Make a copy of the source texture
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

    firstPass = false;
  }

  ZoomFadeIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    auto zoomShaderProgram = GLSL
    (
      110,
      uniform sampler2D texture;
      uniform sampler2D texture2;
      uniform float progress;

      vec2 zoom(vec2 uv, float amount) {
        return 0.5 + ((uv - 0.5) * (1.0 - amount));
      }

      void main() {
        gl_FragColor = mix(
          texture2D(texture, zoom(gl_TexCoord[0].xy, smoothstep(0.0, 0.75, progress))),
          texture2D(texture2, gl_TexCoord[0].xy),
          smoothstep(0.55, 1.0, progress)
        );
      }
    );
 
    shader.loadFromMemory(zoomShaderProgram, sf::Shader::Fragment);
  }

  ~ZoomFadeIn() { ; }
};
