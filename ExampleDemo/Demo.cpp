//This file contains the C++ entry main function and demonstrates
//using the Activity Controller with SFML's main loop

#include <SFML/Window.hpp>
#include <Swoosh/ActivityController.h>
#include <Segues/ZoomOut.h>
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

  // For mobile devices or low-end GPU's, you can request optimized effects any time
  app.optimizeForPerformance(true);

  // Add the Main Menu Scene as the first and only scene in our stack
  // This is our starting point for the user
  // app.push<MainMenuScene>(); // <-- uncomment to see a simple push

  // Swoosh now supports generating blank activities from window contents!
  // The segue will copy the window and use it as part of the screen transition
  // as demonstrated here
  app.push<segue<ZoomOut>::to<MainMenuScene>>();

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
