#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Game.h>
#include <Swoosh/Ease.h>

using namespace swoosh;

/**
  @class HorizontalOpen
  @brief Splits the screen into upper and lower halfs and then moves the pieces up and down as if opening

  The effect is the same across all optimized modes
*/
class HorizontalOpen : public Segue {
private:
  sf::Vector2u windowSize;

public:
 void onDraw(IRenderer& renderer) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1.0);

    this->drawLastActivity(renderer);

    renderer.display(); // flip and ready the buffer

    sf::Texture temp(renderer.getTexture()); // Make a copy of the source texture

    sf::Sprite top(temp); 
    top.setTextureRect(sf::IntRect(0, 0, windowSize.x, (int)(windowSize.y / 2.0)));
    top.setPosition(0.0f, (float)(-alpha * top.getTextureRect().height));

    sf::Sprite bottom(temp);
    bottom.setTextureRect(sf::IntRect(0, (int)(windowSize.y / 2.0), windowSize.x, windowSize.y));
    bottom.setPosition(0.0f, (float)(windowSize.y/2.0f) +  ((float)alpha * (bottom.getTextureRect().height-bottom.getTextureRect().top)));

    renderer.clear();

    this->drawNextActivity(renderer);

    renderer.display(); // flip and ready the buffer
    sf::Texture temp2(renderer.getTexture());
    sf::Sprite right(temp2);

    renderer.submit(Immediate(&right));
    renderer.submit(Immediate(&top));
    renderer.submit(Immediate(&bottom));
  }

  HorizontalOpen(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */ 
    windowSize = getController().getVirtualWindowSize();
  }

  ~HorizontalOpen() { }
};
