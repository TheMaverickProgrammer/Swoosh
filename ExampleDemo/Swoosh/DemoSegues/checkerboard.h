#pragma once
#include <Swoosh\Segue.h>
#include <Swoosh\Ease.h>
#include <Swoosh\EmbedGLSL.h>
#include "..\ResourcePaths.h"
#include "..\TextureLoader.h"

using namespace swoosh;

auto CHECKERBOARD_FRAG_SHADER = GLSL 
(
  110,
  uniform sampler2D texture;
  uniform sampler2D map;
  uniform float progress;

  void main()
  {
    vec4 pixel = texture2D(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y));
    vec4 transition = texture2D(map, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y));
    vec4 color = gl_Color * pixel;

    if (progress >= transition.r) {
      color = vec4(1, 1, 1, 0);
    }

    gl_FragColor = color;
  }
);

class Checkerboard : public Segue {
private:
  sf::Texture* temp;
  sf::Texture* pattern;
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

    sf::Sprite left(*temp);
    shader.setUniform("progress", (float)alpha);

    sf::RenderStates states;
    states.shader = &shader;
  
    surface.clear(sf::Color::Transparent);
    surface.draw(left, states);
    surface.display();

    sf::Texture* temp2 = new sf::Texture(surface.getTexture()); // Make a copy of the source texture
    left = sf::Sprite(*temp2);
   
    surface.clear(sf::Color::Transparent);

    this->drawNextActivity(surface);

    surface.display(); // flip and ready the buffer
    sf::Sprite right(surface.getTexture());

    getController().getWindow().draw(right);
    getController().getWindow().draw(left);

    delete temp2;
    surface.clear(sf::Color::Transparent);
  }

  Checkerboard(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    temp = nullptr;
    shader.loadFromMemory(CHECKERBOARD_FRAG_SHADER, sf::Shader::Fragment);
    pattern = loadTexture(SHADER_PATTERN_PATH);
    shader.setUniform("texture", sf::Shader::CurrentTexture);
    shader.setUniform("map", *pattern);
  }

  virtual ~Checkerboard() { ; }
};
