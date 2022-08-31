#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Ease.h>
#include <Swoosh/Game.h>
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Shaders.h>

using namespace swoosh;

/**
  @class PageTurn
  @brief Divides the screen into vertices that acts like a turning page, revealing the next sceen
  @warning Even when mobile optimization is used, may choke on mobile hardware due to SFML bottlenecks

  If optimized for mobile, will capture the scenes once and use less vertices to increase performance on weak hardware
*/
class PageTurn : public Segue {
private:
  glsl::PageTurn shader;
  sf::Texture last, next;
  bool firstPass{ true };

  const int cellsize(const quality& mode) {
    switch (mode) {
    case quality::realtime:
      return 10;
    case quality::reduced:
      return 50;
    }
    
    // quality::mobile
    return 100;
  }

public:
 void onDraw(IRenderer& renderer) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);
    const bool optimized = getController().getRequestedQuality() == quality::mobile;
    const bool useShader = getController().isShadersEnabled();

    sf::Texture temp, temp2;

    renderer.clear(this->getLastActivityBGColor());

    if (firstPass || !optimized) {
      this->drawLastActivity(renderer);

      renderer.display(); // flip and ready the buffer
      last = temp = sf::Texture(renderer.getTexture()); // Make a copy of the source texture
    }
    else {
      temp = last;
    }

    shader.setTexture(&temp);
    shader.setAlpha((float)alpha);

    if(useShader) {
      shader.apply(renderer);
    }

    renderer.display(); 

    sf::Texture copy(renderer.getTexture());
    sf::Sprite left(copy); // Make a copy of the effect to render later

    renderer.clear(this->getNextActivityBGColor());

    if (firstPass || !optimized) {
      this->drawNextActivity(renderer);

      renderer.display(); // flip and ready the buffer
      next = temp2 = sf::Texture(renderer.getTexture());
    }
    else {
      temp2 = next;
    }

    sf::Sprite right(temp2);

    renderer.submit(right);
    renderer.submit(left);

    firstPass = false;
  }

  PageTurn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next),
    shader(getController().getVirtualWindowSize(), cellsize(getController().getRequestedQuality()))
  {
    /* ... */
  }

  ~PageTurn() { ; }
};