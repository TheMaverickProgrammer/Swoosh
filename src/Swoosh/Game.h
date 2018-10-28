#pragma once

#include <SFML/Graphics.hpp>

#define M_PI 3.14159265358979323846  /* pi */

namespace swoosh {
  namespace game {
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

    template<typename T, typename V>
    static double angleTo(T& a, V& b) {
      double angle = atan2(a.y - b.y, a.x - b.x);

      angle = angle * (180.0 / M_PI);

      if (angle < 0) {
        angle = 360.0 - (-angle);
      }

      return angle;
    }

    template<typename T>
    static T normalize(T input) {
      double length = sqrtf(input.x*input.x + input.y * input.y);
      input.x /= length;
      input.y /= length;
      return input;
    }

    template<typename T, typename U, typename V>
    static sf::Vector2<T> direction(U target, V dest) {
      T x = T(target.x - dest.x);
      T y = T(target.y - dest.y);
      sf::Vector2<T> val = normalize(sf::Vector2<T>( x, y ));
      return val;
    }

    void setOrigin(sf::Sprite& sprite, double fx, double fy) {
      sprite.setOrigin(sf::Vector2f(sprite.getGlobalBounds().width * fx, sprite.getGlobalBounds().height * fy));
    }

    void setOrigin(sf::Text& text, double fx, double fy) {
      text.setOrigin(sf::Vector2f(text.getGlobalBounds().width * fx, text.getGlobalBounds().height * fy));
    }

    void drawToScale(sf::RenderTexture& surface, sf::RenderWindow& window, sf::Sprite& sprite) {
      /*sf::Vector2f original = sprite.getPosition();
      sf::Vector2f pixel = window.mapPixelToCoords(sf::Vector2i((int)original.x, (int)original.y));
      sprite.setPosition(pixel);
      surface.draw(sprite);
      sprite.setPosition(original);*/
      sf::Vector2f original = sprite.getPosition();
      sf::Vector2i pixel = window.mapCoordsToPixel(original);
      sprite.setPosition(sf::Vector2f(pixel.x, pixel.y));
      surface.draw(sprite);
      sprite.setPosition(original);
    }

    void drawToScale(sf::RenderTexture& surface, sf::RenderWindow& window, sf::Text& text) {
      /*sf::Vector2f original = text.getPosition();
      sf::Vector2f pixel = window.mapPixelToCoords(sf::Vector2i((int)original.x, (int)original.y));
      text.setPosition(pixel);
      surface.draw(text);
      text.setPosition(original);*/
      sf::Vector2f original = text.getPosition();
      sf::Vector2i pixel = window.mapCoordsToPixel(original);
      text.setPosition(sf::Vector2f(pixel.x, pixel.y));
      surface.draw(text);
      text.setPosition(original);
    }
  }
}