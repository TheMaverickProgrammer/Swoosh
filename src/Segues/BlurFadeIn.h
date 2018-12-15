#pragma once
#include <Swoosh\EmbedGLSL.h>
#include <Swoosh\Segue.h>
#include <Swoosh\Ease.h>

using namespace swoosh;

namespace {
  /*
  fast pass implementation based from https://www.shadertoy.com/view/XdfGDH
  Modified and improved for swoosh
  */
  auto BLUR_SHADER = GLSL
  (
    130,
    uniform sampler2D texture;

    float normpdf(float x, float sigma)
    {
      return 0.39894*exp(-0.5*x*x / (sigma*sigma)) / sigma;
    }


    void main()
    {
      vec3 c = texture2D(texture, gl_TexCoord[0].xy).rgb;
      
      const int mSize = 11;
      const int kSize = const int((float(mSize) - 1.0) / 2.0);
      float kernel[mSize];
      vec3 final_color = vec3(0.0);

      // Create the kernel
      // Increase sigma per 10 multiples of power; this emulates a more powerful blur
      // At no additional cost
      float sigma = 7.0;
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
          final_color += kernel[kSize + j] * kernel[kSize + i] * texture2D(texture, (gl_TexCoord[0].xy + vec2(float(i), float(j))) / textureSize(texture, 0).xy).rgb;
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

    if (elapsed <= duration * 0.5)
      this->drawLastActivity(surface);
    else
      this->drawNextActivity(surface);

    surface.display(); // flip and ready the buffer

    if (temp) delete temp;
    temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    sf::Sprite sprite(*temp);

    surface.clear();

    shader.setUniform("texture", *temp);
    //shader.setUniform("power", (float)alpha * 100.f);
    sf::RenderStates states;
    states.shader = &shader;

    surface.clear(sf::Color::Transparent);
    surface.draw(sprite, states);
  }

  BlurFadeIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */
    temp = nullptr;
    shader.loadFromMemory(::BLUR_SHADER, sf::Shader::Fragment);

  }

  virtual ~BlurFadeIn() { ; }
};
