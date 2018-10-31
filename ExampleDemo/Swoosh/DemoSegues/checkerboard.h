#pragma once
#include <Swoosh\Segue.h>
#include <Swoosh\Ease.h>
#include <Swoosh\EmbedGLSL.h>

using namespace swoosh;

auto CHECKERBOARD_FRAG_SHADER = GLSL 
(
  110,
  uniform sampler2D texture;
  uniform sampler2D pattern;
  uniform float progress;

  void main()
  {
    vec4 pixel = texture2D(texture, vec2(gl_TexCoord[0].xy));
    vec4 transition = texture2D(pattern, vec2(gl_TexCoord[0].xy));
    vec4 color = gl_Color * pixel;

    if (progress >= transition.g) {
      color = vec4(1, 1, 1, 0);
    }

    gl_FragColor = color;
  }
);

class Checkerboard : public Segue {
private:
  sf::Texture* temp;
  sf::Texture pattern;
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

    /* exported from GIMP */
    static const struct {
      unsigned int 	 width;
      unsigned int 	 height;
      unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
      unsigned char	 pixel_data[8 * 6 * 4 + 1];
    } checkerboard_raw_32bit_rgba = {
      8, 6, 4,
      "LKL\377\\\\]\377\276\276\276\377qqq\377dcd\377UWU\377\010\010\010\377---\377"
      "*)*\377\202\202\202\377uuu\377\016\016\016\377YXY\377\322\322\322\377>=>\377"
      "\060\060\060\377\206\206\207\377\037\037\037\377kjk\377\346\346\346\377hgh\377"
      "BAB\377ffg\377ZZZ\377EFE\377nnn\377VVW\377SSS\377|||\377\067\067\067\377\220"
      "\220\220\377\034\034\033\377\377\377\377\377ddd\377a`a\377IHI\377:;;\377\000\000"
      "\000\377xxy\377\021\022\021\377OPO\377&&&\377\247\247\247\377?>?\377\061\061\061"
      "\377###\377\025\025\025\377HHH\377",
    };



    sf::Image buffer;
    buffer.create(checkerboard_raw_32bit_rgba.width, checkerboard_raw_32bit_rgba.height, checkerboard_raw_32bit_rgba.pixel_data);
    pattern.loadFromImage(buffer);

    shader.loadFromMemory(CHECKERBOARD_FRAG_SHADER, sf::Shader::Fragment);
    shader.setUniform("texture", sf::Shader::CurrentTexture);
    shader.setUniform("pattern", pattern);
  }

  virtual ~Checkerboard() { ; }
};