#pragma once
#include <Swoosh\EmbedGLSL.h>
#include <Swoosh\Segue.h>
#include <Swoosh\Ease.h>

using namespace swoosh;

namespace {
  /*
  fast pass implementation based from https://www.shadertoy.com/view/XdfGDH
  Modified and improved for Swoosh
  */
  auto BLUR_SHADER = GLSL
  (
    110,
    uniform sampler2D texture;
    uniform float power;
    uniform float textureSizeW;
    uniform float textureSizeH;

    float normpdf(float x, float sigma)
    {
      return 0.39894*exp(-0.5*x*x / (sigma*sigma)) / sigma;
    }

    void main()
    {
      vec3 c = texture2D(texture, gl_TexCoord[0].xy).rgb;
      
      const int mSize = 60;
      const int kSize = int((float(mSize) - 1.0) / 2.0);
      float kernel[mSize];
      vec3 final_color = vec3(0.0);

      // Create the kernel
      // Increase sigma per 10 multiples of power; this emulates a more powerful blur
      // At no additional cost
      float sigma = 1.0 + power;
      float Z = 0.0;
      for (int j = 0; j <= kSize; ++j)
      {
        kernel[kSize + j] = kernel[kSize - j] = normpdf(float(j), sigma);
      }

      //get the normalization factor (as the gaussian has been clamped)
      for (int j = 0; j < mSize; ++j)
      {
        Z += kernel[j];
      }

      //read out the texels
      for (int i = -kSize; i <= kSize; ++i)
      {
        for (int j = -kSize; j <= kSize; ++j)
        {
          final_color += kernel[kSize + j] * kernel[kSize + i] * texture2D(texture, (gl_TexCoord[0].xy + (vec2(float(i), float(j)) / vec2(textureSizeW, textureSizeH)))).rgb;
        }
      }

      gl_FragColor = vec4(final_color / (Z*Z), 1.0);
    }
  );
}
class BlurFadeIn : public Segue {
private:
  sf::Texture* temp;
  sf::Shader shader;

public:
  virtual void onDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::wideParabola(elapsed, duration, 1.0);

    this->drawLastActivity(surface);

    surface.display(); // flip and ready the buffer

    if (temp) delete temp;
    temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    sf::Sprite sprite(*temp);

    surface.clear(sf::Color::Transparent);

    shader.setUniform("texture", *temp);
    shader.setUniform("power", (float)alpha * 10.f);
    shader.setUniform("textureSizeW", (float)temp->getSize().x);
    shader.setUniform("textureSizeH", (float)temp->getSize().y);

    sf::RenderStates states;
    states.shader = &shader;

    surface.draw(sprite, states);

    surface.display();
    sf::Texture* last = new sf::Texture(surface.getTexture());

    surface.clear(sf::Color::Transparent);

    this->drawNextActivity(surface);

    surface.display(); // flip and ready the buffer

    delete temp;
    temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    sf::Sprite sprite2 = sf::Sprite(*temp);

    surface.clear(sf::Color::Transparent);

    shader.setUniform("texture", *temp);
    shader.setUniform("power", (float)alpha * 10.f);
    shader.setUniform("textureSizeW", (float)temp->getSize().x);
    shader.setUniform("textureSizeH", (float)temp->getSize().y);

    states.shader = &shader;

    surface.draw(sprite2, states);

    surface.display();
    sf::Texture* next = new sf::Texture(surface.getTexture());

    sprite.setTexture(*last);
    sprite2.setTexture(*next);

    surface.clear(sf::Color::Transparent);

    sf::RenderWindow& window = getController().getWindow();

    alpha = ease::linear(elapsed, duration, 1.0);

    sprite.setColor(sf::Color(255, 255, 255, (sf::Uint8)(255.0 * (1-alpha))));
    sprite2.setColor(sf::Color(255, 255, 255, (sf::Uint8)(255.0 * alpha)));

    window.draw(sprite);
    window.draw(sprite2);
    
    delete last;
    delete next;
  }

  BlurFadeIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    temp = nullptr;
    shader.loadFromMemory(::BLUR_SHADER, sf::Shader::Fragment);

  }

  virtual ~BlurFadeIn() { ; }
};
