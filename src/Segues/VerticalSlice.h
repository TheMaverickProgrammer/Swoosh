#pragma once
#include <Swoosh/Segue.h>
#include <Swoosh/Game.h>
#include <Swoosh/Ease.h>

using namespace swoosh;

/**
  @class VerticalSlice
  @brief Slices the screen in half and moves the pieces in opposite vertical directions revealing the next scene behind them

  Behavior is the same across all quality modes
*/
class VerticalSlice : public Segue {
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
    static sf::Texture temp(renderer.getTexture()); // Make a copy of the source texture

    static sf::Sprite left(temp); 
    left.setTextureRect(sf::IntRect(0, 0, (int)(windowSize.x/2.0), windowSize.y));
    left.setPosition(0, (float)(direction * alpha * (double)left.getTexture()->getSize().y));

    static sf::Sprite right(temp);
    right.setTextureRect(sf::IntRect((int)(windowSize.x/2.0), 0, windowSize.x, windowSize.y));
    right.setPosition((float)(windowSize.x/2.0f), (float)(direction * -alpha * (double)right.getTexture()->getSize().y));

    renderer.clear();

    this->drawNextActivity(renderer);

    renderer.display(); // flip and ready the buffer

    static sf::Texture temp2(renderer.getTexture());
    static sf::Sprite next(temp2);

    renderer.submit(next);
    renderer.submit(left);
    renderer.submit(right);
  }

  VerticalSlice(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) {
    /* ... */ 
    windowSize = getController().getVirtualWindowSize();
    direction = rand() % 2 == 0 ? -1 : 1;
  }

  ~VerticalSlice() { }
};
