#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Game.h>
#include <Swoosh/Ease.h>

using namespace swoosh;


/**
  @class VerticalOpen
  @brief Slices the screen in half vertical and moves the pieces left and right revealing the next scene behind them

  Behavior is the same across all quality modes
*/

class VerticalOpen : public Segue {
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

    sf::Sprite left(temp); 
    left.setTextureRect(sf::IntRect(0, 0, (int)(windowSize.x/2.0f), windowSize.y));
    left.setPosition((float)-alpha * (float)left.getTextureRect().width, 0.0f);

    sf::Sprite right(temp);
    right.setTextureRect(sf::IntRect((int)(windowSize.x/2.0f), 0, windowSize.x, windowSize.y));
    right.setPosition((float)(windowSize.x/2.0f) + ((float)alpha * (right.getTextureRect().width-right.getTextureRect().left)), 0.0f);

    renderer.clear();

    this->drawNextActivity(renderer);

    renderer.display(); // flip and ready the buffer
    sf::Texture temp2(renderer.getTexture());
    sf::Sprite next(temp2);

    renderer.submit(Immediate(next));
    renderer.submit(Immediate(left));
    renderer.submit(Immediate(right));
  }

  VerticalOpen(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */ 
    windowSize = getController().getVirtualWindowSize();
  }

  ~VerticalOpen() {}
};
