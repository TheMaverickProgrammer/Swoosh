#pragma once
#include "ActivityController.h"
#include <SFML\Graphics.hpp>
#include "TextureLoader.h"
#include "Particle.h"

class DemoScene : public Activity {
private:
  sf::Texture* bgTexture;
  sf::Sprite bg;

  sf::Font   menuFont;
  sf::Text   menuText;

public:
  DemoScene(ActivityController& controller) : Activity(controller) { ; }

  virtual void OnStart() {
    bgTexture = LoadTexture("resources/bg.png");
    bg = sf::Sprite(*bgTexture);

    menuFont.loadFromFile("resources/commando.ttf");
    menuText.setFont(menuFont);

    menuText.setFillColor(sf::Color::Red);
  }

  virtual void OnUpdate(double elapsed) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
      controller.QueuePop();
    }
  }

  virtual void OnLeave() {

  }

  virtual void OnResume() {

  }

  virtual void OnDraw(sf::RenderTexture& surface) {
    surface.draw(bg);

    menuText.setPosition(sf::Vector2f(200, 100));
    menuText.setString("Press \"S\" to go back");
    surface.draw(menuText);
  }

  virtual void OnEnd() {
    delete bgTexture;
  }

  virtual ~DemoScene() { ; }
};