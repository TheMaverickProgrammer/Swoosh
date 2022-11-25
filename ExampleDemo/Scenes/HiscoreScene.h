#pragma once
#include "../TextureLoader.h"
#include "../Particle.h"
#include "../Button.h"
#include "../ResourcePaths.h"
#include "../SaveFile.h"

#include <Swoosh/ActivityController.h>
#include <Swoosh/Game.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Segues/SlideIn.h>
#include <Segues/CircleClose.h>
#include <Segues/RetroBlit.h>
#include <iostream>

using namespace swoosh;
using namespace swoosh::types;

class MainMenuScene;

class HiScoreScene : public Activity {
private:
  sf::Texture * meteorBig, * meteorMed, * meteorSmall, * meteorTiny, * btn;

  std::vector<particle> meteors;
  button goback;

  sf::Font   font;
  sf::Text   text;

  sf::SoundBuffer buffer;
  sf::Sound selectFX;

  save& hiscore;

  int lives;

  float screenDiv;
  float screenMid;
  float screenBottom;

  Timer waitTime;
  double scrollOffset;

  bool inFocus;
public:
  HiScoreScene(ActivityController& controller, save& data) : hiscore(data), Activity(&controller) {
    // Proof that this is the same save file in memory as it is passed around the scenes
    std::cout << "savefile address is " << &data << std::endl;

    // keep our window size dimensions consistent based on the initial window size when the AC was created
    auto windowSize = getController().getVirtualWindowSize();
    setView(windowSize);

    font.loadFromFile(GAME_FONT);
    text.setFont(font);
    text.setFillColor(sf::Color::White);

    btn = loadTexture(BLUE_BTN_PATH);
    goback.sprite = sf::Sprite(*btn);
    goback.text = "Return";

    meteorBig = loadTexture(METEOR_BIG_PATH);
    meteorMed = loadTexture(METEOR_MED_PATH);
    meteorSmall = loadTexture(METEOR_SMALL_PATH);
    meteorTiny = loadTexture(METEOR_TINY_PATH);

    screenBottom = (float)windowSize.y;
    screenMid = windowSize.x / 2.0f;
    screenDiv = windowSize.x / 4.0f;

    if (hiscore.empty()) {
      hiscore.writeToFile(SAVE_FILE_PATH);
      hiscore.loadFromFile(SAVE_FILE_PATH);
    }

    waitTime.start();
    scrollOffset = 0;

    // Load sounds
    buffer.loadFromFile(SHIELD_UP_SFX_PATH);
    selectFX.setBuffer(buffer);

    inFocus = false;

    this->setBGColor(sf::Color::Black);
  }

  void onStart() override {
    inFocus = true;
  }

  void onUpdate(double elapsed) override {
    waitTime.update(sf::seconds((float)elapsed));

    goback.update(getController().getWindow());

    if (goback.isClicked && inFocus) {
      selectFX.play();

      // Rewind lets us pop back to a particular scene in our stack history
      using effect = segue<CircleClose, sec<1>>;
      bool found = getController().rewind<effect::to<MainMenuScene>>();

      // should never happen
      // but your games may need to check so here it is an example
      assert(found && "MainMenuScene not found in our running game");
    }

    // After 3 seconds, scroll up
    if (waitTime.getElapsed().asSeconds() > 3) {

      // If the scroll offset is greater than the height of all drawn scores
      if (scrollOffset > 200 + (hiscore.names.size() * 100)) {
        // We hit them all, reset
        scrollOffset = 0;
        waitTime.reset();
      }
      else {
        scrollOffset += 100.0 * elapsed;
      }
    }

    for (auto& m : meteors) {
      sf::Vector2f prevPos = m.pos;
      m.pos += sf::Vector2f(m.speed.x * (float)elapsed, m.speed.y * (float)elapsed);
      m.sprite.setPosition(m.pos);
      m.sprite.setRotation(m.pos.x);
    }
  }

  void onLeave() override {
    inFocus = false;
  }

  void onExit() override {
  }

  void onEnter() override {
    for (int i = 50; i > 0; i--) {
      int randNegativeX = rand() % 2 == 0 ? -1 : 1;
      int randNegativeY = rand() % 2 == 0 ? -1 : 1;

      int randSpeedX = rand() % 40;
      randSpeedX *= randNegativeX;

      int randSpeedY = rand() % 40;
      randSpeedY *= randNegativeY;

      particle p;

      int randTexture = rand() % 4;

      switch (randTexture) {
      case 0:
        p.sprite = sf::Sprite(*meteorBig);
        break;
      case 1:
        p.sprite = sf::Sprite(*meteorMed);
        break;
      case 2:
        p.sprite = sf::Sprite(*meteorSmall);
        break;
      default:
        p.sprite = sf::Sprite(*meteorTiny);
      }

      p.pos = sf::Vector2f((float)(rand() % getController().getWindow().getSize().x), (float)(rand() % getController().getWindow().getSize().y));
      p.sprite.setPosition(p.pos);
      p.sprite.setRotation(p.pos.x);

      p.speed = sf::Vector2f((float)randSpeedX, (float)randSpeedY);
      meteors.push_back(p);
    }
  }

  void onResume() override {
  }

  void onDraw(IRenderer& renderer) override {
    sf::RenderWindow& window = getController().getWindow();

    for (auto& m : meteors) {
      renderer.submit(&m.sprite);
    }

    text.setFillColor(sf::Color::Yellow);
    text.setPosition(sf::Vector2f(screenMid, 100));
    text.setString("Hi Scores");
    setOrigin(text, 0.5, 0.5);
    renderer.submit(Immediate(&text));

    text.setFillColor(sf::Color::White);

    for (int i = 0; i < hiscore.names.size(); i++) {
      std::string name = hiscore.names[i];
      int score = hiscore.scores[i];

      text.setString(name);
      text.setPosition(sf::Vector2f((float)(screenDiv), (float)(200 + (i*100) - scrollOffset)));
      setOrigin(text, 0.5, 0.5);
      renderer.submit(Immediate(&text));

      text.setString(std::to_string(score));
      text.setPosition(sf::Vector2f((float)(screenDiv * 3), (float)(200 + (i*100) - scrollOffset)));
      setOrigin(text, 0.5, 0.5);
      renderer.submit(Immediate(&text));
    }

    text.setFillColor(sf::Color::Black);
    goback.draw(renderer, text, screenMid, screenBottom - 40);
  }

  void onEnd() override {
  }

  ~HiScoreScene() { delete btn; }
};
