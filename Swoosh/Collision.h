#pragma once

#include <SFML/Graphics.hpp>

static bool doesCollide(sf::Sprite& a, sf::Sprite& b) {
  double mx = a.getPosition().x;
  double my = a.getPosition().y;
  double mw = a.getGlobalBounds().width;
  double mh = a.getGlobalBounds().height;

  double mx2 = b.getPosition().x;
  double my2 = b.getPosition().y;
  double mw2 = b.getGlobalBounds().width;
  double mh2 = b.getGlobalBounds().height;

  return (mx < mx2 + mw2 && mx + mw > mx2 && my < my2 + mh2 && my + mh > my2);
}