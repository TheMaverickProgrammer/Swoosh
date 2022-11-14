#pragma once

#include "GameplayScene.h"
#include "HiscoreScene.h"
#include "AboutScene.h"
#include "../CustomRenderer.h"
#include "../TextureLoader.h"
#include "../Particle.h"
#include "../Button.h"
#include "../SaveFile.h"

// You can use any of the included segues in the actions below
// to see what they look like! Start at line 170 in this source file!
#include <Segues/BlackWashFade.h>
#include <Segues/CrossZoom.h>
#include <Segues/ZoomFadeIn.h>
#include <Segues/ZoomFadeInBounce.h>
#include <Segues/Checkerboard.h>
#include <Segues/WhiteWashFade.h>
#include <Segues/SlideIn.h>
#include <Segues/BlendFadeIn.h>
#include <Segues/PageTurn.h>
#include <Segues/ZoomOut.h>
#include <Segues/ZoomIn.h>
#include <Segues/HorizontalSlice.h>
#include <Segues/VerticalSlice.h>
#include <Segues/HorizontalOpen.h>
#include <Segues/VerticalOpen.h>
#include <Segues/PixelateBlackWashFade.h>
#include <Segues/BlurFadeIn.h>
#include <Segues/SwipeIn.h>
#include <Segues/DiamondTileSwipe.h>
#include <Segues/DiamondTileCircle.h>
#include <Segues/CircleOpen.h>
#include <Segues/CircleClose.h>
#include <Segues/Morph.h>
#include <Segues/RadialCCW.h>
#include <Segues/Cube3D.h>
#include <Segues/RetroBlit.h>
#include <Segues/Dream.h> // 10/10/2020
// end segue effects

#include <Swoosh/ActivityController.h>
#include <Swoosh/Game.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

#define GAME_TITLE   "Swoosh Interactive Demo"
#define PLAY_OPTION  "Play"
#define SCORE_OPTION "HiScore"
#define ABOUT_OPTION "About"

using namespace swoosh::types;

class MainMenuScene : public Activity {
private:
  sf::Texture* bgTexture, *bgNormal;
  sf::Texture* starTexture;
  sf::Texture* blueButton, *redButton, *greenButton;

  sf::Sprite bg;

  sf::Font menuFont;
  sf::Text menuText;

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
  MainMenuScene(ActivityController& controller) : Activity(&controller) {
    setView(controller.getVirtualWindowSize());

    savefile.loadFromFile(SAVE_FILE_PATH);

    inFocus = true;
    fadeMusic = false;

    bgTexture = loadTexture(MENU_BG_PATH);
    bgNormal = loadTexture(MENU_BG_N_PATH);
    bg = sf::Sprite(*bgTexture);

    starTexture = loadTexture(STAR_PATH);

    blueButton = loadTexture(BLUE_BTN_PATH);
    redButton = loadTexture(RED_BTN_PATH);
    greenButton = loadTexture(GREEN_BTN_PATH);

    menuFont.loadFromFile(GAME_FONT);

    menuText.setFont(menuFont);
    menuText.setFillColor(sf::Color::White);

    screenMid = getController().getWindow().getSize().x / 2.0f;

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

    timer.start();

    this->setBGColor(sf::Color(56, 7, 67));
  }

  void onStart() override {
    std::cout << "MainMenuScene OnStart called" << std::endl;
    themeMusic.play();
  }

  void onUpdate(double elapsed) override {
    timer.update(sf::seconds((float)elapsed));

    if (!inFocus && fadeMusic) {
      themeMusic.setVolume(themeMusic.getVolume() * 0.98f); // fades out the music
    }

    int i = 0;
    for (auto& p : particles) {
      p.speed = sf::Vector2f(p.speed.x * p.friction.x, p.speed.y * p.friction.y);
      p.pos += sf::Vector2f(p.speed.x * (float)elapsed, p.speed.y * (float)elapsed);

      p.sprite.setPosition(p.pos);
      p.sprite.setScale(2.0f*static_cast<float>(p.life / p.lifetime), 2.0f*static_cast<float>(p.life / p.lifetime));

      auto color = p.sprite.getColor();
      color.a = (sf::Uint8)(255.0 * (p.life / p.lifetime));

      p.sprite.setColor(color);

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
          using segue = segue<HorizontalOpen>;
          using intent = segue::to<GameplayScene>;
          getController().push<intent>(savefile);
          fadeMusic = true;
        }
        else if (b.text == SCORE_OPTION) {
          using segue = segue<RadialCCW, sec<2>>;
          using intent = segue::to<HiScoreScene>;
          getController().push<intent>(savefile);
        }
        else if (b.text == ABOUT_OPTION) {
          using segue = segue<PageTurn, sec<2>>;
          using intent = segue::to<AboutScene>;
          getController().push<intent>();
        }
      }
    }
  }

  void onLeave() override {
    std::cout << "MainMenuScene OnLeave called" << std::endl;
    inFocus = false;
  }

  void onExit() override {
    std::cout << "MainMenuScene OnExit called" << std::endl;

    if (fadeMusic) {
      themeMusic.stop();
    }
  }

  void onEnter() override {
    std::cout << "MainMenuScene OnEnter called" << std::endl;

  }


  void onResume() override {
    inFocus = true;

    // If fadeMusic == true, then we were coming from demo, the music changes
    if (fadeMusic) {
      themeMusic.play();
      themeMusic.setVolume(100);
    }

    fadeMusic = false;

    std::cout << "MainMenuScene OnResume called" << std::endl;

    for (int i = 50; i > 0; i--) {
      int randNegative = rand() % 2 == 0 ? -1 : 1;
      int randSpeedX = rand() % 80;
      randSpeedX *= randNegative;
      int randSpeedY = rand() % 220;

      particle p;
      p.sprite = sf::Sprite(*starTexture);
      p.pos = sf::Vector2f((float)(rand() % getController().getVirtualWindowSize().x), (float)(getController().getVirtualWindowSize().y));
      p.speed = sf::Vector2f((float)randSpeedX, (float)-randSpeedY);
      p.friction = sf::Vector2f(0.99999f, 0.9999f);
      p.life = 3.0;
      p.lifetime = 3.0;
      p.sprite.setPosition(p.pos);
      setOrigin(p.sprite, 0.5, 0.5);

      particles.push_back(p);
    }
  }

  void onDraw(IRenderer& renderer) override {
    const bool isCustomRenderer = getController().getCurrentRendererName() == "custom";
    renderer.submit(Fake3D(bg, bgNormal));

    for (auto& p : particles) {
      renderer.submit(p.sprite);
    }

    int i = 0;
    menuText.setFillColor(sf::Color::Black);

    for (auto& b : buttons) {
      b.draw(renderer, menuText, screenMid, (float)(200 + (i++*100)));
    }

    // First set the text as the it would render as a full string
    menuText.setString(GAME_TITLE);
    setOrigin(menuText, 0.5, 0.5);

    // -30 is a made up number offset to help it look right
    menuText.setPosition(sf::Vector2f(screenMid-30.f, 100));

    // Get the global bounds information from that to pick out
    double startX = menuText.getGlobalBounds().left;
    double offset = 0;

    // For each letter in the string, make it jump while preserving placement
    size_t len = strlen(GAME_TITLE);
    double frequency = swoosh::ease::pi * 2.0 / len;
    double dt = timer.getElapsed().asSeconds();
    for (int i = 0; i < len; i++) {
      menuText.setFillColor(sf::Color::White);
      menuText.setString(GAME_TITLE[i]);
      setOrigin(menuText, 0.5, 0.5); // origin is in the center of the letter

      // This creates our wave over all letters
      double ratio = (ease::pi) / len;
      double wave = (std::sin(timer.getElapsed().asSeconds()*2.0+((i+1)*ratio)));

      // Only add the peaks
      double startY = 100 + ((wave > 0)? -wave * 50.0 : 0);

      // Each letter is separate so add back the spacing
      const float letterSpacing = static_cast<float>(menuText.getCharacterSize()) / 3.0f;

      // We want at max 24.f units of space inbetween letters
      // Everything else can be smaller
      offset += std::fminf(menuText.getGlobalBounds().width + letterSpacing, 24.f);

      // Include spaces
      if (menuText.getString() == ' ') { offset += menuText.getCharacterSize(); }

      menuText.setPosition(sf::Vector2f((float)(startX + offset), (float)startY));
      renderer.submit(Immediate(menuText));


      if (isCustomRenderer) {

        sf::Uint8 r = ((sin(frequency * i + 2 + dt) + 1.0) / 2.0) * 255U;
        sf::Uint8 g = ((sin(frequency * i + 0 + dt) + 1.0) / 2.0) * 255U;
        sf::Uint8 b = ((sin(frequency * i + 4 + dt) + 1.0) / 2.0) * 255U;

        renderer.submit(Light(160.0, WithZ(menuText.getPosition(), 50.0f), sf::Color(r, g, b, 255), 0.1f));
      }
    }
  }

  void onEnd() override {
    std::cout << "MainMenuScene OnEnd called" << std::endl;

    while (!particles.empty()) {
      particles.erase(particles.begin());
    }
  }

  ~MainMenuScene() {
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
