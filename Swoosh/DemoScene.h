#pragma once
#include "ActivityController.h"
#include <SFML\Graphics.hpp>
#include "TextureLoader.h"
#include "Particle.h"
#include "ResourcePaths.h"

#include "Checkerboard.h"

#include <iostream>

using namespace swoosh;

#define M_PI         3.14159265358979323846  /* pi */

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

  sf::Font   font;
  sf::Text   text;

  sf::SoundBuffer laserFX;
  sf::Sound speakers;
  sf::Music ingameMusic;

  int lives;
  double alpha;

  bool mousePressed;
  bool mouseRelease;
  bool inFocus;
public:
  DemoScene(ActivityController& controller) : Activity(controller) { 
    mousePressed = mouseRelease = inFocus = false;

    ingameMusic.openFromFile(INGAME_MUSIC_PATH);
    laserFX.loadFromFile(LASER1_SFX_PATH);

    bgTexture = loadTexture(PURPLE_BG_PATH);
    bgTexture->setRepeated(true);
    bg = sf::Sprite(*bgTexture);
    bg.setTextureRect({ 0, 0, 800, 600 });

    meteorBig = loadTexture(METEOR_BIG_PATH);
    meteorMed = loadTexture(METEOR_MED_PATH);
    meteorSmall = loadTexture(METEOR_SMALL_PATH);
    meteorTiny = loadTexture(METEOR_TINY_PATH);

    laserTexture = loadTexture(LASER_BEAM_PATH);
    shieldTexture = loadTexture(SHIELD_LOW_PATH);
    enemyTexture = loadTexture(ENEMY_PATH);

    playerTexture = loadTexture(PLAYER_PATH);
    player.sprite = sf::Sprite(*playerTexture);
    player.sprite.setOrigin(sf::Vector2f(player.sprite.getGlobalBounds().width / 2.0, player.sprite.getGlobalBounds().height / 2.0));

    // start is in the center of the screen
    sf::RenderWindow& window = getController().getWindow();
    player.pos = sf::Vector2f(window.getSize().x / 2.0, window.getSize().y / 2.0);
    alpha = 0;

    font.loadFromFile(GAME_FONT);
    text.setFont(font);

    text.setFillColor(sf::Color::White); 
  }

  virtual void onStart() {
    std::cout << "DemoScene OnStart called" << std::endl;
    ingameMusic.play();
    inFocus = true;
  }

  virtual void onUpdate(double elapsed) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
      getController().queuePop<ActivityController::Segue<Checkerboard, Duration<&sf::seconds, 3>>>();
    }

    for (auto& m : meteors) {
      sf::Vector2f prevPos = m.pos;
      m.pos += sf::Vector2f(m.speed.x * elapsed, m.speed.y * elapsed);
      m.sprite.setPosition(m.pos);
      m.sprite.setRotation(m.pos.x);
    }

    int i = 0;
    for (auto& t : trails) {
      t.sprite.setPosition(t.pos);
      t.sprite.setScale((t.life / t.lifetime), (t.life / t.lifetime));
      t.sprite.setColor(sf::Color(t.sprite.getColor().r, t.sprite.getColor().g, t.sprite.getColor().b, 10 * t.life / t.lifetime));
      t.life -= elapsed;

      if (t.life < 0) {
        trails.erase(trails.begin() + i);
        continue;
      }

      i++;
    }

    i = 0;
    for (auto& l : lasers) {
      l.pos.x += l.speed.x * elapsed;
      l.pos.y += l.speed.y * elapsed;
      l.sprite.setPosition(l.pos);
      l.life -= elapsed;

      //double ratio = 2*l.life / l.lifetime;

      //l.sprite.setColor(sf::Color(ratio*l.sprite.getColor().r, ratio*l.sprite.getColor().g, ratio*l.sprite.getColor().b, 255));

      if (l.life < 0) {
        lasers.erase(lasers.begin() + i);
        continue;
      }

      i++;
    }

    sf::RenderWindow& window = getController().getWindow();
    double angle = atan2(sf::Mouse::getPosition(window).y - player.pos.y, sf::Mouse::getPosition(window).x - player.pos.x);

    angle = angle * (180.0 / M_PI);

    if (angle < 0){
      angle = 360.0 - (-angle);
    }

    player.sprite.setRotation(90.0 + angle);

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
      sf::Vector2f delta = player.pos;
      sf::Vector2i m = sf::Mouse::getPosition(window);

      sf::Vector2f direction = sf::Vector2f(m.x - delta.x, m.y - delta.y);
      double length = sqrtf(direction.x*direction.x + direction.y * direction.y);
      direction.x /= length;
      direction.y /= length;

      delta.x = direction.x * 5 * elapsed;
      delta.y = direction.y * 5 * elapsed;
      player.speed += delta;

      particle trail = player;
      trail.life = trail.lifetime = 1.0; // secs
      trails.push_back(trail);
    }

    player.speed.x = std::min(50.f, player.speed.x);
    player.speed.x = std::max(-50.f, player.speed.x);
    player.speed.y = std::min(50.f, player.speed.y);
    player.speed.y = std::max(-50.f, player.speed.y);

    player.pos += player.speed;

    player.pos.x = std::min(player.pos.x, (float)window.getView().getSize().x);
    player.pos.x = std::max(0.0f, player.pos.x);
    player.pos.y = std::min(player.pos.y, (float)window.getView().getSize().y);
    player.pos.y = std::max(0.0f, player.pos.y);

    player.sprite.setPosition(player.pos);

    alpha += 100 * elapsed;

    alpha = std::min(alpha, 255.0);
    player.sprite.setColor(sf::Color(255, 255, 255, alpha));

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && inFocus) {
      if (!mousePressed) {
        particle laser;
        laser.sprite = sf::Sprite(*laserTexture);
        laser.sprite.setOrigin(sf::Vector2f(laser.sprite.getGlobalBounds().width / 2.0, laser.sprite.getGlobalBounds().height / 2.0));
        laser.pos = player.pos;
        laser.sprite.setRotation(90.0f + angle);
        laser.sprite.setPosition(laser.pos);

        sf::Vector2f delta = laser.pos;
        sf::Vector2i m = sf::Mouse::getPosition(window);

        sf::Vector2f direction = sf::Vector2f(m.x - delta.x, m.y - delta.y);
        double length = sqrtf(direction.x*direction.x + direction.y * direction.y);
        direction.x /= length;
        direction.y /= length;

        delta.x = direction.x * 500.0f;
        delta.y = direction.y * 500.0f;
        laser.speed = delta;

        lasers.push_back(laser);

        speakers.setBuffer(laserFX);
        speakers.play();

        mousePressed = true;
      }
    }
    else {
      mousePressed = false;
    }
  }

  virtual void onLeave() {
    std::cout << "DemoScene OnLeave called" << std::endl;
    ingameMusic.stop();
    inFocus = false;
  }

  virtual void onExit() {
    std::cout << "DemoScene OnExit called" << std::endl;
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

      p.pos = sf::Vector2f(rand() % getController().getWindow().getSize().x, rand() % getController().getWindow().getSize().y);
      p.sprite.setPosition(p.pos);

      p.speed = sf::Vector2f(randSpeedX, randSpeedY);
      meteors.push_back(p);
    }
  }

  virtual void onResume() {
    std::cout << "DemoScene OnResume called" << std::endl;

  }

  virtual void onDraw(sf::RenderTexture& surface) {
    surface.draw(bg);

    for (auto& t : trails) {
      surface.draw(t.sprite);
    }

    for (auto& m : meteors) {
      surface.draw(m.sprite);
    }

    for (auto& l : lasers) {
      surface.draw(l.sprite);
    }
    
    text.setPosition(sf::Vector2f(300, 100));
    text.setString("Survive");
    surface.draw(text);

    surface.draw(player.sprite);
  }

  virtual void onEnd() {
    std::cout << "DemoScene OnEnd called" << std::endl;
  }

  virtual ~DemoScene() { delete bgTexture;; }
};