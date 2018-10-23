#pragma once
#include <SFML/Graphics.hpp>

struct particle {
  sf::Sprite sprite;
  sf::Vector2f pos;
  sf::Vector2f speed;
  sf::Vector2f friction;
  double life;
  double lifetime;
};