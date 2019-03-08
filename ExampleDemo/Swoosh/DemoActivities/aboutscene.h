#pragma once
#include <Swoosh\ActivityController.h>
#include <Swoosh\Game.h>
#include <SFML\Graphics.hpp>
#include <SFML\Audio.hpp>
#include "TextureLoader.h"
#include "Particle.h"
#include "Button.h"
#include "ResourcePaths.h"
#include "SaveFile.h"

#include <Segues\PushIn.h>
#include <Segues\BlendFadeIn.h>
#include <Segues\Cube3D.h>

#include <iostream>

#define TEXT_BLOCK_INFO "Swoosh is an Activity and Segue mini library\n" \
                        "designed to make complex screen transitions\n" \
                        "a thing of the past.\n" \
                        "This is a proof-of-concept demo showcasing\n" \
                        "its features and includes helpful utilities\n" \
                        "for your SFML apps or games.\n\n" \
                        "Fork at\ngithub.com/TheMaverickProgrammer/Swoosh"

#define CONTROLS_INFO ">> Left click to shoot\n\n"\
                      ">> Right click to boost and dodge\n\n" \
                      ">> Collect stars for extra life\n\n"

using namespace swoosh;
using namespace swoosh::intent;

class AboutScene : public Activity {
private:
  sf::Texture * btn;
  sf::Texture * sfmlTexture;
  sf::Sprite sfml;
  button goback;

  sf::Font   manual;
  sf::Font   font;
  sf::Text   text;
  std::string info;

  sf::SoundBuffer buffer;
  sf::Sound selectFX;

  float screenDiv;
  float screenMid;
  float screenBottom;

  Timer timer;

  bool inFocus;
  bool canClick;
public:
  AboutScene(ActivityController& controller) : Activity(&controller) {
    canClick = false;

    font.loadFromFile(GAME_FONT);
    text.setFont(font);
    text.setFillColor(sf::Color::White);

    manual.loadFromFile(MANUAL_FONT);

    btn = loadTexture(BLUE_BTN_PATH);
    goback.sprite = sf::Sprite(*btn);
    goback.text = "Continue";
    info = TEXT_BLOCK_INFO;

    sfmlTexture = loadTexture(SFML_PATH);
    sfml = sf::Sprite(*sfmlTexture);
    sfml.setScale(0.7f, 0.7f);
    setOrigin(sfml, 0.5f, 0.60f);

    sf::Vector2u windowSize = getController().getVirtualWindowSize();

    screenBottom = (float)windowSize.y;
    screenMid = windowSize.x / 2.0f;
    screenDiv = windowSize.y / 4.0f;

    // Load sounds
    buffer.loadFromFile(SHIELD_UP_SFX_PATH);
    selectFX.setBuffer(buffer);

    inFocus = false;

    timer.start();
  }

  virtual void onStart() {
    inFocus = true;
  }

  virtual void onUpdate(double elapsed) {

    double offset = 0;
    if (timer.getElapsed().asSeconds() > 3) {

      offset = ease::wideParabola(timer.getElapsed().asMilliseconds()-3000.0, 5000.0, 0.9);
    }

    sf::Vector2u windowSize = getController().getVirtualWindowSize();

    sfml.setPosition(100.0f + (float)(offset * (windowSize.x - 300)), 100.0f);
    sfml.setRotation((float)(offset * 360 * 2));

    goback.update(getController().getWindow());

    if (goback.isClicked && inFocus) {
      if (canClick) {
        canClick = false;
        selectFX.play();

        if (goback.text == "FIN") {
          getController().queuePop<segue<Cube3D<direction::right>, sec<5>>>();
        }
        else {
          goback.text = "FIN";
          info = CONTROLS_INFO;
        }
      }

    }

    if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !canClick) {
      canClick = true;
    }
  }

  virtual void onLeave() {
    inFocus = false;
  }

  virtual void onExit() {
  }

  virtual void onEnter() {
    
  }

  virtual void onResume() {
  }

  virtual void onDraw(sf::RenderTexture& surface) {
    sf::RenderWindow& window = getController().getWindow();

    surface.clear(sf::Color::Black);

    surface.draw(sfml);

    text.setFont(manual);
    text.setPosition(sf::Vector2f(screenMid, 200));
    text.setFillColor(sf::Color::White);
    text.setString(info);
    setOrigin(text, 0.5f, 0);

    surface.draw(text);

    text.setFont(font);
    text.setFillColor(sf::Color::Black);
    setOrigin(text, 0.5f, 0.5f);
    goback.draw(surface, text, screenMid, screenBottom - 40);
  }

  virtual void onEnd() {
  }

  virtual ~AboutScene() { delete btn; }
};