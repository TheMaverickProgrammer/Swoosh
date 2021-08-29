#pragma once
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/Shaders.h>

using namespace swoosh;

/**
  @class Dream
  @brief Wiggles the current scene into the next scene like a dream sequence
  @param wiggle_power. Compile-time constant to increase the intensity of the wiggle. Values from 0 => 100

  If optimized for mobile, will capture the scenes once to increase performance on weak hardware
*/
template<int wiggle_power>
class DreamCustom : public Segue {
private:
  std::string shaderProgram;
  sf::Shader shader;
  sf::Texture last, next;
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
      surface.clear(this->getLastActivityBGColor());
      this->drawLastActivity(surface);

      surface.display(); // flip and ready the buffer
      last = temp = sf::Texture(surface.getTexture()); // Make a copy of the source texture
    }
    else {
      temp = last;
    }

    if (firstPass || !optimized) {
      surface.clear(this->getNextActivityBGColor());
      this->drawNextActivity(surface);

      surface.display(); // flip and ready the buffer
      next = temp2 = sf::Texture(surface.getTexture()); // Make a copy of the source texture
    }
    else {
      temp2 = next;
    }

    shader.setUniform("texture", temp);
    shader.setUniform("texture2", temp2);
    shader.setUniform("alpha", (float)alpha);

    sf::RenderStates states;
    
    if(useShader) {
      states.shader = &shader;
    }

    sf::Sprite sprite(temp2); // dummy. we just need something with the screen size to draw with
    surface.draw(sprite, states);

    firstPass = false;
  }

 DreamCustom(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    shaderProgram = GLSL(110,
        uniform float alpha;
        uniform int power;

        uniform sampler2D texture;
        uniform sampler2D texture2;

        vec2 offset(float progress, float x) {
          return vec2(0, 0.03 * progress * cos(float(power) * (progress + x)));
        }
        
        void main() {
          vec2 pos = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);
          gl_FragColor = mix(texture2D(texture, pos + offset(alpha, pos.x)), texture2D(texture2, pos + offset(1.0-alpha, pos.x)), alpha);
        }
    );

    shader.loadFromMemory(shaderProgram, sf::Shader::Fragment);
    shader.setUniform("power", wiggle_power);
  }

  ~DreamCustom() { }
};

//!< Custom shorthand to use a dream with wiggle_power=10.0
using Dream = DreamCustom<10>;
