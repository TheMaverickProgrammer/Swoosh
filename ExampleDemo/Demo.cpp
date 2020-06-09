// This file contains the 'main' function. Program execution begins and ends there.
//
#include <SFML/Window.hpp>
#include <Swoosh/ActivityController.h>
#include "Scenes/MainMenuScene.h"

using namespace swoosh;

int main()
{
  sf::RenderWindow window(sf::VideoMode(800, 600), "Swoosh Demo");
  window.setFramerateLimit(60); // call this once, after creating the window
  window.setVerticalSyncEnabled(true);
  window.setMouseCursorVisible(false);

  // Create an AC with the current window as our target to draw to
  ActivityController app(window);

  // Add the Main Menu Scene as the first and only scene in our stack
  // This is our starting point for the user
  app.push<MainMenuScene>();

  sf::Texture* cursorTexture = loadTexture(CURSOR_PATH);
  sf::Sprite cursor;

  cursor.setTexture(*cursorTexture);

  // run the program as long as the window is open
  float elapsed = 0.0f;
  sf::Clock clock;
  bool pause = false;

  srand((unsigned int)time(0));

  while (window.isOpen())
  {
    clock.restart();

    // check all the window's events that were triggered since the last iteration of the loop
    sf::Event event;
    while (window.pollEvent(event))
    {
      // "close requested" event: we close the window
      if (event.type == sf::Event::Closed) {
        window.close();
      } else if (event.type == sf::Event::LostFocus) {
        pause = true;
      }
      else if (event.type == sf::Event::GainedFocus) {
        pause = false;
      }
    }

    window.clear();

    // do not update segues when the window is frozen
    if (!pause) {
      app.update(elapsed);
    }

    // draw() will directly draw onto the window's render buffer
    app.draw();

    sf::Vector2f mousepos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    cursor.setPosition(mousepos);

    // Draw the mouse cursor over everything else
    window.draw(cursor);

    // Display to our screen
    window.display();

    elapsed = static_cast<float>(clock.getElapsedTime().asSeconds());
  }

  delete cursorTexture;

  return 0;
}