#pragma once
#pragma once
#include "ActivityController.h"
#include <SFML\Graphics.hpp>
#include <SFML\Audio.hpp>
#include "TextureLoader.h"
#include "ResourcePaths.h"
#include <iostream>

using namespace swoosh;

class GameOverScene : public Activity {
private:
  sf::Texture* bgTexture;
  sf::Sprite bg;
  sf::SoundBuffer buffer;
  sf::Sound gameover;
public:
  GameOverScene(ActivityController& controller) : Activity(controller) {
    bgTexture = loadTexture(GAME_OVER_PATH);
    bg = sf::Sprite(*bgTexture);

    buffer.loadFromFile(LOSE_SFX_PATH);
    gameover.setBuffer(buffer);
  }

  virtual void onStart() { 
    // The first time we see this screen is the only time we see this screen
    gameover.play();
  }

  virtual void onUpdate(double elapsed) { }
  virtual void onLeave() { }

  virtual void onExit() {}

  virtual void onEnter() {
  }

  virtual void onResume() {}

  virtual void onDraw(sf::RenderTexture& surface) {
    surface.draw(bg);
  }

  virtual void onEnd() {}

  virtual ~GameOverScene() { delete bgTexture;; }
};