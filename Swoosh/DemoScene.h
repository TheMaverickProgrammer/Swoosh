#pragma once
#include "ActivityController.h"
#include <SFML\Graphics.hpp>
#include "TextureLoader.h"
#include "Particle.h"

#include "SlideIn.h"

#include <iostream>

using namespace swoosh;

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

  virtual void onStart() {
    std::cout << "DemoScene OnStart called" << std::endl;
  }

  virtual void onUpdate(double elapsed) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
      controller.queuePop<ActivityController::Segue<SlideIn>>();
    }
  }

  virtual void onLeave() {
    std::cout << "DemoScene OnLeave called" << std::endl;

  }

  virtual void onExit() {
    std::cout << "DemoScene OnExit called" << std::endl;
  }

  virtual void onEnter() {
    std::cout << "DemoScene OnEnter called" << std::endl;
  }

  virtual void onResume() {
    std::cout << "DemoScene OnResume called" << std::endl;

  }

  virtual void onDraw(sf::RenderTexture& surface) {
    surface.draw(bg);

    menuText.setPosition(sf::Vector2f(200, 100));
    menuText.setString("Press \"S\" to go back");
    surface.draw(menuText);
  }

  virtual void onEnd() {
    std::cout << "DemoScene OnEnd called" << std::endl;
  }

  virtual ~DemoScene() { delete bgTexture;; }
};