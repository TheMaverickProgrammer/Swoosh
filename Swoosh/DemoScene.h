#pragma once
#include "ActivityController.h"
#include <SFML\Graphics.hpp>
#include "TextureLoader.h"
#include "Particle.h"

#include "SlideIn.h"

#include <iostream>

class DemoScene : public Activity {
private:
  sf::Texture* bgTexture;
  sf::Sprite bg;

  sf::Font   menuFont;
  sf::Text   menuText;

public:
  DemoScene(ActivityController& controller) : Activity(controller) { 
    bgTexture = LoadTexture("resources/scene.png");
    bg = sf::Sprite(*bgTexture);

    menuFont.loadFromFile("resources/commando.ttf");
    menuText.setFont(menuFont);

    menuText.setFillColor(sf::Color::Red); 
  }

  virtual void OnStart() {
    std::cout << "DemoScene OnStart called" << std::endl;
  }

  virtual void OnUpdate(double elapsed) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
      controller.QueuePop<ActivityController::Segue<SlideIn>>();
    }
  }

  virtual void OnLeave() {
    std::cout << "DemoScene OnLeave called" << std::endl;

  }

  virtual void OnResume() {
    std::cout << "DemoScene OnResume called" << std::endl;

  }

  virtual void OnDraw(sf::RenderTexture& surface) {
    surface.draw(bg);

    menuText.setPosition(sf::Vector2f(200, 100));
    menuText.setString("Press \"S\" to go back");
    surface.draw(menuText);
  }

  virtual void OnEnd() {
    std::cout << "DemoScene OnEnd called" << std::endl;
  }

  virtual ~DemoScene() { delete bgTexture;; }
};