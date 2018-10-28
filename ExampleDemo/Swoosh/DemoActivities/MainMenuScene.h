#pragma once
#include <Swoosh\ActivityController.h>
#include <Swoosh\Game.h>
#include <SFML\Graphics.hpp>
#include <SFML\Audio.hpp>

#include "DemoSegues\WhiteWashFade.h"
#include "DemoSegues\SlideIn.h"
#include "DemoSegues\BlendFadeIn.h"
#include "DemoScene.h"
#include "HiScoreScene.h"
#include "AboutScene.h"
#include "TextureLoader.h"
#include "Particle.h"
#include "Button.h"
#include "SaveFile.h"

#include <iostream>

#define GAME_TITLE   "Swoosh Interactive Demo"
#define PLAY_OPTION  "Play"
#define SCORE_OPTION "HiScore"
#define ABOUT_OPTION "About"

using namespace swoosh::intent;

class MainMenuScene : public Activity {
private:
  sf::Texture* bgTexture;
  sf::Texture* starTexture;
  sf::Texture * blueButton, * redButton, * greenButton;

  sf::Sprite bg;

  sf::Font   menuFont;
  sf::Text   menuText;

  sf::SoundBuffer buffer;
  sf::Sound selectFX;
  sf::Music themeMusic;

  std::vector<particle> particles;
  std::vector<button> buttons;

  float screenMid;
  bool inFocus;
  bool fadeMusic;

  Timer timer; // for onscreen effects. Or we could have stored the total elapsed from the update function
  save savefile;
public:
  MainMenuScene(ActivityController& controller) : Activity(controller) { 
    savefile.loadFromFile(SAVE_FILE_PATH);

    inFocus = true;
    fadeMusic = false;

    bgTexture = loadTexture(MENU_BG_PATH);
    bg = sf::Sprite(*bgTexture);

    starTexture = loadTexture(STAR_PATH);

    blueButton = loadTexture(BLUE_BTN_PATH);
    redButton = loadTexture(RED_BTN_PATH);
    greenButton = loadTexture(GREEN_BTN_PATH);

    menuFont.loadFromFile(GAME_FONT);
    menuText.setFont(menuFont);

    menuText.setFillColor(sf::Color::White); 

    screenMid = getController().getWindow().getSize().x / 2.0;

    // Create the buttons
    button menuOption;
    menuOption.sprite.setTexture(*blueButton);
    menuOption.text = PLAY_OPTION;
    buttons.push_back(menuOption); 

    menuOption.sprite.setTexture(*redButton);
    menuOption.text = SCORE_OPTION;
    buttons.push_back(menuOption);

    menuOption.sprite.setTexture(*greenButton);
    menuOption.text = ABOUT_OPTION;
    buttons.push_back(menuOption);

    // Load sounds
    buffer.loadFromFile(SHIELD_UP_SFX_PATH);
    selectFX.setBuffer(buffer);
    themeMusic.openFromFile(THEME_MUSIC_PATH);

    timer.reset();
  }

  virtual void onStart() {
    std::cout << "MainMenuScene OnStart called" << std::endl;
    themeMusic.play();
  }

  virtual void onUpdate(double elapsed) {
    if (!inFocus && fadeMusic) {
      themeMusic.setVolume(themeMusic.getVolume() * 0.90); // quieter
    }

    int i = 0;
    for (auto& p : particles) {
      p.speed = sf::Vector2f(p.speed.x * p.friction.x, p.speed.y * p.friction.y);
      p.pos += sf::Vector2f(p.speed.x * elapsed, p.speed.y * elapsed);

      p.sprite.setPosition(p.pos);
      p.sprite.setScale(2.0*(p.life / p.lifetime), 2.0*(p.life / p.lifetime));
      p.sprite.setColor(sf::Color(p.sprite.getColor().r, p.sprite.getColor().g, p.sprite.getColor().b, 255 * p.life / p.lifetime));
      p.life -= elapsed;

      if (p.life <= 0) {
        particles.erase(particles.begin() + i);
        continue;
      }

      i++;
    }

    for (auto& b : buttons) {
      b.update(getController().getWindow());

      if (b.isClicked && inFocus) {
        selectFX.play();

        if (b.text == PLAY_OPTION) {
          getController().push<segue<WhiteWashFade, sec<1>>::to<DemoScene>>(savefile);
          fadeMusic = true;
        }
        else if (b.text == SCORE_OPTION) {
          getController().push<segue<SlideIn, sec<1>>::to<HiScoreScene>>(savefile);
        }
        else if (b.text == ABOUT_OPTION) {
          getController().push<segue<BlendFadeIn, sec<2>>::to<AboutScene>>();
        }
      }
    }
  }

  virtual void onLeave() {
    std::cout << "MainMenuScene OnLeave called" << std::endl;
    inFocus = false;
  }

  virtual void onExit() {
    std::cout << "MainMenuScene OnExit called" << std::endl;

    if (fadeMusic) {
      themeMusic.stop();
    }
  }

  virtual void onEnter() {
    std::cout << "MainMenuScene OnEnter called" << std::endl;

  }


  virtual void onResume() {
    inFocus = true;

    // We were coming from demo, the music changes
    if (fadeMusic) {
      themeMusic.play();
      themeMusic.setVolume(100);
    }

    fadeMusic = false;

    std::cout << "MainMenuScene OnResume called" << std::endl;

    for (int i = 100; i > 0; i--) {
      int randNegative = rand() % 2 == 0 ? -1 : 1;
      int randSpeedX = rand() % 80;
      randSpeedX *= randNegative;
      int randSpeedY = rand() % 220;

      particle p;
      p.sprite = sf::Sprite(*starTexture);
      p.pos = sf::Vector2f(rand() % getController().getWindow().getSize().x, getController().getWindow().getSize().y);
      p.speed = sf::Vector2f(randSpeedX, -randSpeedY);
      p.friction = sf::Vector2f(0.99999f, 0.9999f);
      p.life = 3.0;
      p.lifetime = 3.0;
      p.sprite.setPosition(p.pos);

      particles.push_back(p);
    }
  }

  virtual void onDraw(sf::RenderTexture& surface) {
    sf::RenderWindow& window = getController().getWindow();

    surface.draw(bg);

    for (auto& p : particles) {
      drawToScale(surface, window, p.sprite);
    }

    int i = 0;
    menuText.setFillColor(sf::Color::Black);
    for (auto& b : buttons) {
      b.drawToScale(surface, window, menuText, screenMid, 200 + (i++*100));
    }

    // First set the text as the it would render as a full string
    menuText.setString(GAME_TITLE);
    setOrigin(menuText, 0.5, 0.5);
    menuText.setPosition(sf::Vector2f(screenMid, 100));

    // Get the global bounds information from that to pick out
    double startX = menuText.getGlobalBounds().left;
    double offset = 0;

    // For each letter in the string, make it jump while preserving placement
    for (int i = 0; i < strlen(GAME_TITLE); i++) {
      menuText.setFillColor(sf::Color::White);
      menuText.setString(GAME_TITLE[i]);
      setOrigin(menuText, 0.5, 0.5);

      // This creates our wave over all letters
      double ratio = (M_PI) / strlen(GAME_TITLE);
      double wave = (std::sin(timer.getElapsed().asSeconds()*2.0+((i+1)*ratio)));

      // Only add the peaks
      double startY = 100 + ((wave > 0)? -wave * 50.0 : 0);

      // Each letter is separate so add back the spacing
      offset += menuText.getGlobalBounds().width;

      // Include spaces
      if (menuText.getString() == ' ') { offset += menuText.getCharacterSize(); }

      menuText.setPosition(sf::Vector2f(startX + offset, startY));
      drawToScale(surface, window, menuText);
    }
  }

  virtual void onEnd() {
    std::cout << "MainMenuScene OnEnd called" << std::endl;

    while (!particles.empty()) {
      particles.erase(particles.begin());
    }
  }

  virtual ~MainMenuScene() {
    delete bgTexture;

    while (!particles.empty()) {
      particles.erase(particles.begin());
    }

    delete starTexture;
    delete blueButton;
    delete greenButton;
    delete redButton;

    savefile.writeToFile(SAVE_FILE_PATH);
  }
};