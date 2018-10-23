#pragma once
#include "ActivityController.h"
#include <SFML\Graphics.hpp>
#include "TextureLoader.h"
#include "Particle.h"

#include "DemoScene.h"
#include "WhiteWashFade.h"

class MainMenuScene : public Activity {
private:
  sf::Texture* bgTexture;
  sf::Texture* cursorTexture;
  sf::Texture* starTexture;

  sf::Sprite bg;
  sf::Sprite cursor;

  sf::Font   menuFont;
  sf::Text   menuText;

  std::vector<particle> particles;

public:
  MainMenuScene(ActivityController& controller) : Activity(controller) { ; }

  virtual void OnStart() {
    bgTexture = LoadTexture("resources/bg.png");
    bg = sf::Sprite(*bgTexture);

    cursorTexture = LoadTexture("resources/cursor.png");
    cursor = sf::Sprite(*cursorTexture);

    starTexture = LoadTexture("resources/star.png");

    menuFont.loadFromFile("resources/commando.ttf");
    menuText.setFont(menuFont);

    menuText.setFillColor(sf::Color::Red);
  }

  virtual void OnUpdate(double elapsed) {
    for (auto p : particles) {
      p.speed = sf::Vector2f(p.speed.x * p.friction.x, p.speed.y * p.friction.y);
      p.pos += sf::Vector2f(p.speed.x * elapsed, p.speed.y * elapsed);
      p.sprite.setPosition(p.pos);
      p.sprite.setScale(p.life / p.lifetime, p.life / p.lifetime);
      p.sprite.setColor(sf::Color(p.sprite.getColor().r, p.sprite.getColor().g, p.sprite.getColor().b, p.life / p.lifetime));
      p.life -= elapsed;
    }

    cursor.setPosition(sf::Vector2f(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y));

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
      controller.Push(ActivityController::Segue<WhiteWashFade>::To<DemoScene>());
    }
  }

  virtual void OnLeave() {

  }

  virtual void OnResume() {
    for (int i = 100; i > 0; i--) {
      int randNegative = rand() % 2 == 0 ? -1 : 1;
      int randSpeedX = rand() % 10;
      randSpeedX *= randNegative;
      int randSpeedY = rand() % 11;

      particles.push_back({
        sf::Sprite(*starTexture),
        sf::Vector2f(0, controller.getWindow().getSize().y),
        sf::Vector2f(randSpeedX, -randSpeedY),
        sf::Vector2f(1.0f, 1.0f),
        3 * 1000,
        3 * 1000
        });
    }
  }

  virtual void OnDraw(sf::RenderTexture& surface) {
    surface.draw(bg);

    menuText.setPosition(sf::Vector2f(200, 100));
    menuText.setString("BlendIn");
    surface.draw(menuText);

    menuText.setPosition(sf::Vector2f(200, 200));
    menuText.setString("RealTimeCheckerboard");
    surface.draw(menuText);

    menuText.setPosition(sf::Vector2f(200, 300));
    menuText.setString("WaitUntilComplete");
    surface.draw(menuText);

    menuText.setPosition(sf::Vector2f(200, 400));
    menuText.setString("SlideIn");
    surface.draw(menuText);

    for (auto p : particles) {
      surface.draw(p.sprite);
    }

    surface.draw(cursor);
  }

  virtual void OnEnd() {
    delete bgTexture;
    delete cursorTexture;
    
    while (!particles.empty()) {
      particles.erase(particles.begin());
    }

    delete starTexture;
  }

  virtual ~MainMenuScene() { ; }
};