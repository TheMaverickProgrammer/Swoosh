#pragma once
#include <SFML/Graphics.hpp>

struct particle {
  sf::Sprite sprite;
  sf::Vector2f pos;
  sf::Vector2f speed;
  sf::Vector2f friction;
  double life;
  double lifetime;

  particle() {
    pos = speed = sf::Vector2f(0.0f, 0.0f);
    life = lifetime = 1.0;
    friction = sf::Vector2f(1.0f, 1.0f);
  }

  particle(const particle& rhs) {
    sprite = rhs.sprite;
    pos = rhs.pos;
    speed = rhs.speed;
    friction = rhs.friction;
    life = rhs.life;
    lifetime = rhs.lifetime;
  }
};