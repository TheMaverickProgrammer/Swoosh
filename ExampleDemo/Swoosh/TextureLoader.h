#pragma once
#include <SFML\Graphics.hpp>
#include <string>
static sf::Texture* loadTexture(const std::string& path) {
  sf::Texture* texture = new sf::Texture();
  if (!texture->loadFromFile(path)) {
    throw std::runtime_error("Texture at " + path + " failed to load");
  }

  return texture;
}