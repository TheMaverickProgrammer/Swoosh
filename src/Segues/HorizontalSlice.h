#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Game.h>
#include <Swoosh/Ease.h>

using namespace swoosh;

/**
  @class HorizontalSlice
  @brief Splits the screen into upper and lower halfs and then sends the pieces in opposite directions, revealing the next scene

  The effect is the same across all optimized modes
*/
class HorizontalSlice : public Segue {
private:
  sf::Vector2u windowSize;
  int direction;
public:
 void onDraw(IRenderer& renderer) override {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = 1.0 - ease::bezierPopOut(elapsed, duration);

    this->drawLastActivity(renderer);

    renderer.display(); // flip and ready the buffer

    sf::Texture temp(renderer.getTexture()); // Make a copy of the source texture

    sf::Sprite top(temp); 
    top.setTextureRect(sf::IntRect(0, 0, windowSize.x, windowSize.y / 2));
    top.setPosition((float)(direction * alpha * top.getTexture()->getSize().x), 0.0f);

    sf::Sprite bottom(temp);
    bottom.setTextureRect(sf::IntRect(0, windowSize.y / 2, windowSize.x, windowSize.y));
    bottom.setPosition((float)(direction * -alpha * bottom.getTexture()->getSize().x), (float)(windowSize.y/2.0f));

    renderer.clear();

    this->drawNextActivity(renderer);

    renderer.display(); // flip and ready the buffer

    sf::Texture temp2(renderer.getTexture());
    sf::Sprite right(temp2);

    renderer.submit(right);
    renderer.submit(top);
    renderer.submit(bottom);
  }

  HorizontalSlice(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */ 
    windowSize = getController().getVirtualWindowSize();
    direction = rand() % 2 == 0 ? -1 : 1;
  }

  ~HorizontalSlice() { }
};
