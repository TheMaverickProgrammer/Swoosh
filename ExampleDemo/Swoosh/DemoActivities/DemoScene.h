#pragma once
#include <Swoosh\ActivityController.h>
#include <Swoosh\Game.h>

#include <SFML\Graphics.hpp>
#include "..\TextureLoader.h"
#include "..\Particle.h"
#include "..\ResourcePaths.h"
#include "..\SaveFile.h"

#include <Segues\Checkerboard.h>

#include <iostream>
#include <assert.h>

class HiScoreScene;

using namespace swoosh;
using namespace swoosh::game;
using namespace swoosh::intent;

class DemoScene : public Activity {
private:
  sf::Texture* bgTexture;
  sf::Sprite bg;

  sf::Texture* playerTexture;
  particle player;
  std::vector<particle> trails;

  sf::Texture* enemyTexture;
  std::vector<particle> enemies;

  sf::Texture * meteorBig, *meteorMed, *meteorSmall, *meteorTiny, *btn;
  std::vector<particle> meteors;

  sf::Texture *laserTexture;
  std::vector<particle> lasers;

  sf::Texture * shieldTexture;
  sf::Sprite shield;

  sf::Texture * extraLifeTexture;
  sf::Sprite star;

  sf::Texture * numeralTexture[11];
  sf::Texture * playerLifeTexture;
  sf::Sprite numeral;
  sf::Sprite playerLife;

  sf::Font   font;
  sf::Text   text;

  sf::SoundBuffer laserFX;
  sf::SoundBuffer shieldFX;
  sf::SoundBuffer gameOverFX;
  sf::SoundBuffer extraLifeFX;
  sf::Sound laserChannel;
  sf::Sound shieldChannel;
  sf::Sound extraLifeChannel;
  sf::Sound gameOverChannel;
  sf::Music ingameMusic;

  int lives;
  long score;
  double alpha;
  bool hasShield;
  bool isExtraLifeSpawned;

  bool mousePressed;
  bool mouseRelease;
  bool inFocus;

  save& savefile;
public:
  DemoScene(ActivityController& controller, save& savefile) : savefile(savefile), Activity(&controller) { 
    mousePressed = mouseRelease = inFocus = isExtraLifeSpawned = false;

    ingameMusic.openFromFile(INGAME_MUSIC_PATH);
    ingameMusic.setLoop(true);
    ingameMusic.setLoopPoints(sf::Music::TimeSpan(sf::seconds(0), sf::seconds(49)));

    laserFX.loadFromFile(LASER1_SFX_PATH);
    shieldFX.loadFromFile(SHIELD_DOWN_SFX_PATH);
    gameOverFX.loadFromFile(LOSE_SFX_PATH);
    extraLifeFX.loadFromFile(TWO_TONE_SFX_PATH);

    laserChannel.setBuffer(laserFX);
    shieldChannel.setBuffer(shieldFX);
    gameOverChannel.setBuffer(gameOverFX);
    extraLifeChannel.setBuffer(extraLifeFX);

    sf::Vector2u windowSize = getController().getVirtualWindowSize();

    bgTexture = loadTexture(PURPLE_BG_PATH);
    bgTexture->setRepeated(true);
    bg = sf::Sprite(*bgTexture);
    bg.setTextureRect({ 0, 0, (int)windowSize.x, (int)windowSize.y });

    meteorBig = loadTexture(METEOR_BIG_PATH);
    meteorMed = loadTexture(METEOR_MED_PATH);
    meteorSmall = loadTexture(METEOR_SMALL_PATH);
    meteorTiny = loadTexture(METEOR_TINY_PATH);

    laserTexture = loadTexture(LASER_BEAM_PATH);
    shieldTexture = loadTexture(SHIELD_LOW_PATH);
    enemyTexture = loadTexture(ENEMY_PATH);
    extraLifeTexture = loadTexture(EXTRA_LIFE_PATH);

    star = sf::Sprite(*extraLifeTexture);
    setOrigin(star, 0.5, 0.5);

    playerTexture = loadTexture(PLAYER_PATH);
    player.sprite = sf::Sprite(*playerTexture);
    setOrigin(player.sprite, 0.5, 0.5);

    shield = sf::Sprite(*shieldTexture);
    setOrigin(shield, 0.5, 0.5);

    for (int i = 0; i < 11; i++) {
      numeralTexture[i] = loadTexture(NUMERAL_PATH[i]);
    }

    playerLifeTexture = loadTexture(PLAYER_LIFE_PATH);
    playerLife = sf::Sprite(*playerLifeTexture);

    resetPlayer();

    font.loadFromFile(GAME_FONT);
    text.setFont(font);

    text.setFillColor(sf::Color::White); 

    lives = 3;
    score = 0;
  }

  void spawnEnemy() {
    sf::RenderWindow& window = getController().getWindow();

    particle enemy;
    enemy.sprite = sf::Sprite(*enemyTexture);
    setOrigin(enemy.sprite, 0.5, 0.5);

    sf::Vector2u windowSize = getController().getVirtualWindowSize();

    int side = rand() % 2;
    if(side == 0)
      enemy.pos = sf::Vector2f((float)((rand() % 2) * windowSize.x), (float)(rand() % windowSize.y));
    else 
      enemy.pos = sf::Vector2f((float)(rand() % windowSize.x), (float)((rand() % 2) * windowSize.y));

    enemy.lifetime = 0; // this enemy stays alive until a lifetime is provided
    enemy.sprite.setPosition(enemy.pos);
    enemies.push_back(enemy);
  }

  void resetPlayer() {
    // start is in the center of the screen
    sf::Vector2u windowSize = getController().getVirtualWindowSize();

    player.pos = sf::Vector2f(windowSize.x / 2.0f, windowSize.y / 2.0f);
    player.speed = sf::Vector2f(0, 0);
    player.sprite.setPosition(player.pos);
    player.friction = sf::Vector2f(0.96f, 0.96f);
    alpha = 0;
    hasShield = true;
  }

  virtual void onStart() {
    std::cout << "DemoScene OnStart called" << std::endl;
    ingameMusic.play();
    inFocus = true;
  }

  virtual void onUpdate(double elapsed) {
    sf::RenderWindow& window = getController().getWindow();
    auto windowSize = getController().getVirtualWindowSize();

    if (lives < 0) {
      getController().push<segue<Checkerboard, milli<900>>::to<HiScoreScene>>(savefile);
    }

    for (auto& m : meteors) {
      m.pos += sf::Vector2f(m.speed.x * (float)elapsed, m.speed.y * (float)elapsed);
      m.sprite.setPosition(m.pos);
      m.sprite.setRotation(m.pos.x);
    }

    int i = 0;
    for (auto& t : trails) {
      t.sprite.setPosition(t.pos);
      t.sprite.setScale((float)(t.life / t.lifetime), (float)(t.life / t.lifetime));
      t.sprite.setColor(sf::Color(t.sprite.getColor().r, t.sprite.getColor().g, t.sprite.getColor().b, (sf::Uint8)(10.0f * (t.life / t.lifetime))));
      t.life -= elapsed;

      if (t.life <= 0) {
        trails.erase(trails.begin() + i);
        continue;
      }

      i++;
    }

    i = 0;
    bool killShield = false;

    for (auto& e : enemies) {
      if (e.lifetime == 0) {
        for (auto& l : lasers) {
          if (e.lifetime != 0) break; // Reward player once
          if (doesCollide(l.sprite, e.sprite)) {
            l.life = 0;
            e.lifetime = 1; // trigger scale out
            score += 1000;
          }
        }
      }

      if (e.life <= 0) {
        enemies.erase(enemies.begin() + i);
        continue;
      }

      if (lives >= 0 && e.lifetime == 0) {
        if (alpha >= 255.0 && doesCollide(e.sprite, player.sprite)) {
          if (hasShield && !killShield) {
            shieldChannel.play();
            killShield = true; // give us time to protect from other enemies
            alpha = 100; // invincibility 
          }
          else {
            resetPlayer();
            gameOverChannel.play();
            lives--;

            for (auto& e2 : enemies) {
              e2.lifetime = 1.0; // trigger scale out on ALL enemies
            }
          }

          e.lifetime = 1.0; // trigger scale out on this enemy
        }

        double angle = angleTo(player.pos, e.pos);

        e.sprite.setRotation(90.0f + (float)angle);
        e.sprite.setPosition(e.pos);

        sf::Vector2f dir = directionTo<float>(player.pos, e.pos);
        sf::Vector2f delta;
        delta.x = dir.x * 2.0f;
        delta.y = dir.y * 2.0f;
        e.speed += delta;
      }

      if (e.lifetime > 0) {
        e.life -= 2*elapsed;
        float scale = (float)(e.life / e.lifetime);
        e.sprite.setScale(scale, scale);
        e.speed.x = e.speed.y = 0;
      } 

      e.pos += sf::Vector2f(e.speed.x * (float)elapsed, e.speed.y * (float)elapsed);
      e.sprite.setPosition(e.pos);
      i++;
    }

    i = 0;
    for (auto& l : lasers) {
      l.pos.x += l.speed.x * (float)elapsed;
      l.pos.y += l.speed.y * (float)elapsed;
      l.sprite.setPosition(l.pos);
      l.life -= elapsed;

      double ratio = 3*l.life / l.lifetime;
      ratio = std::min(ratio, 1.0);

      l.sprite.setColor(sf::Color((sf::Uint8)(ratio*l.sprite.getColor().r), (sf::Uint8)(ratio*l.sprite.getColor().g), (sf::Uint8)(ratio*l.sprite.getColor().b), 255));

      if (l.life <= 0) {
        lasers.erase(lasers.begin() + i);
        continue;
      }

      i++;
    }

    if (rand() % 50 == 0 && enemies.size() < 10) {
      spawnEnemy();

      if (rand() % 30 == 0 && !isExtraLifeSpawned) {
        isExtraLifeSpawned = true;

        star.setPosition((float)(rand() % windowSize.x), (float)(rand() % windowSize.y));

        // do not spawn on top of player
        while (doesCollide(star, player.sprite)) {
          star.setPosition((float)(rand() % windowSize.x), (float)(rand() % windowSize.y));
        }
      }
    }

    if (lives < 0) return; // do not update player logic 

    if (isExtraLifeSpawned) {
      if (doesCollide(star, player.sprite)) {
        isExtraLifeSpawned = false;
        extraLifeChannel.play();
        lives = std::min(lives+1, 9);
        score += 250;
      }
    }

    sf::Vector2f mousepos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    double angle = angleTo(mousepos, player.pos);

    player.sprite.setRotation(90.0f + (float)angle);

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
      sf::Vector2f dir = directionTo<float>(mousepos, player.pos);
      sf::Vector2f delta = player.speed;
      delta.x += dir.x * 30.0f * (float)elapsed;
      delta.y += dir.y * 30.0f * (float)elapsed;

      player.speed = delta;

      particle trail = player;
      trail.life = trail.lifetime = 1.0; // secs
      trails.push_back(trail);
    }
    else {
      // apply the brakes
      player.speed.x *= player.friction.x;
      player.speed.y *= player.friction.y;
    }

    player.speed.x = std::min(20.f, player.speed.x);
    player.speed.x = std::max(-20.f, player.speed.x);
    player.speed.y = std::min(20.f, player.speed.y);
    player.speed.y = std::max(-20.f, player.speed.y);

    player.pos += player.speed;

    player.pos.x = std::min(player.pos.x, (float)windowSize.x);
    player.pos.x = std::max(0.0f, player.pos.x);
    player.pos.y = std::min(player.pos.y, (float)windowSize.y);
    player.pos.y = std::max(0.0f, player.pos.y);

    player.sprite.setPosition(player.pos);

    alpha += 50 * elapsed;

    alpha = std::min(alpha, 255.0);
    player.sprite.setColor(sf::Color(255, 255, 255, (sf::Uint8)alpha));

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && inFocus) {
      if (!mousePressed) {
        particle laser;
        laser.sprite = sf::Sprite(*laserTexture);
        setOrigin(laser.sprite, 0.5, 0.5);
        laser.pos = player.pos;
        laser.sprite.setRotation(90.0f + (float)angle);
        laser.sprite.setPosition(laser.pos);

        sf::Vector2f dir = directionTo<float>(mousepos, laser.pos);
        sf::Vector2f delta;
        delta.x = dir.x * 500.0f;
        delta.y = dir.y * 500.0f;
        laser.speed = delta;

        lasers.push_back(laser);

        laserChannel.play();

        mousePressed = true;
      }
    }
    else {
      mousePressed = false;
    }

    if(hasShield && killShield)
      hasShield = false;
  }

  virtual void onLeave() {
    std::cout << "DemoScene OnLeave called" << std::endl;
    ingameMusic.stop();
    inFocus = false;
  }

  virtual void onExit() {
    std::cout << "DemoScene OnExit called" << std::endl;

    savefile.names.insert(savefile.names.begin(), "ZZZ");
    savefile.scores.insert(savefile.scores.begin(), score);
    savefile.writeToFile(SAVE_FILE_PATH);
  }

  virtual void onEnter() {
    std::cout << "DemoScene OnEnter called" << std::endl;

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

      auto windowSize = getController().getVirtualWindowSize();
      p.pos = sf::Vector2f((float)(rand() % windowSize.x), (float)(rand() % windowSize.y));
      p.sprite.setPosition(p.pos);

      p.speed = sf::Vector2f((float)randSpeedX, (float)randSpeedY);
      meteors.push_back(p);
    }
  }

  virtual void onResume() {
    std::cout << "DemoScene OnResume called" << std::endl;

  }

  virtual void onDraw(sf::RenderTexture& surface) {
    sf::RenderWindow& window = getController().getWindow();

    surface.draw(bg);

    for (auto& t : trails) {
      surface.draw(t.sprite);
    }

    for (auto& m : meteors) {
      surface.draw(m.sprite);
    }

    for (auto& e : enemies) {
      surface.draw(e.sprite);
    }

    for (auto& l : lasers) {
      surface.draw(l.sprite);
    }
    
    auto windowSize = getController().getVirtualWindowSize();

    text.setString(std::string("score: ") + std::to_string(score));
    setOrigin(text, 1, 0);
    text.setPosition(sf::Vector2f((float)windowSize.x - 50.0f, 0.0f));

    if (alpha < 255) {
      text.setFillColor(sf::Color::Red);
    }
    else {
      text.setFillColor(sf::Color::White);
    }

    surface.draw(text);

    if (isExtraLifeSpawned) surface.draw(star);


    if (lives >= 0) {
      surface.draw(player.sprite);

      if (hasShield) {
        shield.setPosition(player.pos);
        shield.setRotation(player.sprite.getRotation());
        surface.draw(shield);
      }

      numeral = sf::Sprite(*numeralTexture[10]); // X
      numeral.setPosition(player.pos.x, player.pos.y - 100);
      surface.draw(numeral);

      numeral = sf::Sprite(*numeralTexture[lives]);
      numeral.setPosition(player.pos.x + 20, player.pos.y - 100);
      surface.draw(numeral);

      playerLife.setPosition(player.pos.x - 40, player.pos.y - 100);
      surface.draw(playerLife);
    }
  }

  virtual void onEnd() {
    std::cout << "DemoScene OnEnd called" << std::endl;
  }

  virtual ~DemoScene() { delete bgTexture;; }
};