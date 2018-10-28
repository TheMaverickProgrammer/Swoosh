#pragma once
#include <SFML\Graphics.hpp>
#include <Swoosh\Game.h>

// Custom class definitions
struct button;
const bool isMouseHovering(button& btn, sf::RenderWindow& window);

struct button {
  sf::Sprite sprite;
  std::string text;

  bool isClicked;
  bool isHovering;

  button() { isClicked = isHovering = false;  }
  void update(sf::RenderWindow& window) {
    if (isMouseHovering(*this, window)) {
      isHovering = true;
      isClicked = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    }
    else {
      isHovering = false;
    }
  }

  void draw(sf::RenderTexture& surface, sf::Text& sftext, float x, float y) {
    if (isHovering) {
      sprite.setColor(sf::Color(200, 200, 200));
    }
    else {
      sprite.setColor(sf::Color::White);
    }

    sprite.setPosition(sf::Vector2f(x, y));
    sprite.setOrigin(sprite.getGlobalBounds().width / 2.0f, sprite.getGlobalBounds().height / 2.0f);
    surface.draw(sprite);

    sftext.setString(text);
    sftext.setOrigin(sftext.getGlobalBounds().width / 2.0f, sftext.getGlobalBounds().height / 2.0f);
    sftext.setPosition(sf::Vector2f(x, y - sftext.getGlobalBounds().height / 2.0f));
    surface.draw(sftext);
  }
};

// Custom class definitions
const bool isMouseHovering(button& btn, sf::RenderWindow& window) {
  sf::Sprite& sprite = btn.sprite;
  sf::Vector2i mousei = sf::Mouse::getPosition(window);
  sf::Vector2f mouse = window.mapPixelToCoords(mousei);
  sf::FloatRect bounds = sprite.getGlobalBounds();

  return (mouse.x >= bounds.left && mouse.x <= bounds.left + bounds.width && mouse.y >= bounds.top && mouse.y <= bounds.top + bounds.height);
}