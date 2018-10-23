#pragma once
#include "ActivityController.h"
#include <SFML\Graphics.hpp>
#include "TextureLoader.h"
#include "Particle.h"

#include "DemoScene.h"
#include "WhiteWashFade.h"
#include "SlideIn.h"
#include <iostream>

class MainMenuScene : public Activity {
private:
  sf::Texture* bgTexture;
  sf::Texture* cursorTexture;
  sf::Texture* starTexture;

  sf::Sprite bg;
  sf::Sprite cursor;

  sf::Font   menuFont;
  sf::Text   menuText;

  std::vector<particle*> particles;

public:
  MainMenuScene(ActivityController& controller) : Activity(controller) { 
    bgTexture = LoadTexture("resources/bg.png");
    bg = sf::Sprite(*bgTexture);

    cursorTexture = LoadTexture("resources/cursor.png");
    cursor = sf::Sprite(*cursorTexture);

    starTexture = LoadTexture("resources/star.png");

    menuFont.loadFromFile("resources/commando.ttf");
    menuText.setFont(menuFont);

    menuText.setFillColor(sf::Color::Red); 
  }

  virtual void OnStart() {
    std::cout << "MainMenuScene OnStart called" << std::endl;
  }

  virtual void OnUpdate(double elapsed) {
    int i = 0;
    for (auto ptr : particles) {
      particle& p = *ptr;
      p.speed = sf::Vector2f(p.speed.x * p.friction.x, p.speed.y * p.friction.y);
      p.pos += sf::Vector2f(p.speed.x * elapsed, p.speed.y * elapsed);

      p.sprite.setPosition(p.pos);
      p.sprite.setScale(2.0*(p.life / p.lifetime), 2.0*(p.life / p.lifetime));
      p.sprite.setColor(sf::Color(p.sprite.getColor().r, p.sprite.getColor().g, p.sprite.getColor().b, 255 * p.life / p.lifetime));
      p.life -= elapsed;

      if (p.life < 0) {
        particles.erase(particles.begin() + i);
        continue;
      }

      i++;
    }

    cursor.setPosition(sf::Vector2f(sf::Mouse::getPosition(controller.getWindow()).x, sf::Mouse::getPosition(controller.getWindow()).y));

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
      controller.Push(ActivityController::Segue<WhiteWashFade>::To<DemoScene>());
    } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
      controller.Push(ActivityController::Segue<SlideIn>::To<DemoScene>());
    }
  }

  virtual void OnLeave() {
    std::cout << "MainMenuScene OnLeave called" << std::endl;
  }

  virtual void OnResume() {
    std::cout << "MainMenuScene OnResume called" << std::endl;

    for (int i = 100; i > 0; i--) {
      int randNegative = rand() % 2 == 0 ? -1 : 1;
      int randSpeedX = rand() % 80;
      randSpeedX *= randNegative;
      int randSpeedY = rand() % 220;

      particle* p = new particle();
      p->sprite = sf::Sprite(*starTexture);
      p->pos = sf::Vector2f(rand() % controller.getWindow().getSize().x, controller.getWindow().getSize().y);
      p->speed = sf::Vector2f(randSpeedX, -randSpeedY);
      p->friction = sf::Vector2f(0.99999f, 0.9999f);
      p->life = 3.0;
      p->lifetime = 3.0;

      particles.push_back(p);
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

    for (auto ptr : particles) {
      surface.draw(ptr->sprite);
    }

    surface.draw(cursor);
  }

  virtual void OnEnd() {
    std::cout << "MainMenuScene OnEnd called" << std::endl;

    while (!particles.empty()) {
      delete particles[0];
      particles.erase(particles.begin());
    }
  }

  virtual ~MainMenuScene() {
    delete bgTexture;
    delete cursorTexture;

    while (!particles.empty()) {
      delete particles[0];
      particles.erase(particles.begin());
    }

    delete starTexture;
  }
};