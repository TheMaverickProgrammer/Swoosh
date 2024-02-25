//This file contains the C++ entry main function and demonstrates
//using the Activity Controller with SFML's main loop
#include <Swoosh/ActivityController.h>
#include <Swoosh/Renderers/SimpleRenderer.h>
#include <Segues/ZoomOut.h>
#include <SFML/Window.hpp>
#include "Scenes/MainMenuScene.h"
#include "CustomRenderer.h"

using namespace swoosh;

int main()
{
  sf::RenderWindow window(sf::VideoMode(800, 600), "Swoosh Demo");
  window.setFramerateLimit(60); // call this once, after creating the window
  window.setVerticalSyncEnabled(true);
  window.setMouseCursorVisible(false);

  // 11/23/2022 (NEW BEHAVIOR!)
  // Swoosh now enables custom render pipelines and
  // can switch between them in real-time
  SimpleRenderer simple(window.getView());
  CustomRenderer custom(window.getView());
  RendererEntries renderOptions = { { "simple", simple }, { "custom", custom } };

    // Create an AC with the current window as our target to draw to
  ActivityController app(window, renderOptions);

  // 10/9/2020 
  // For mobile devices or low-end GPU's, you can request optimized 
  // effects any time by setting the performance quality to
  // one of the following: { realtime, reduced, mobile }
  app.optimizeForPerformance(quality::realtime); 
  // app.optimizeForPerformance(quality::mobile); // <-- uncomment me!

  // (DEFAULT BEHAVIOR!)
  // Add the Main Menu Scene as the first and only scene in our stack
  // This is our starting point for the user
  // app.push<MainMenuScene>(); // <-- uncomment to see a simple push

  // 10/9/2020 (NEW BEHAVIOR!)
  // Swoosh now supports generating blank activities from window contents!
  // The segue will copy the window at startup and use it as part of 
  // the screen transition as demonstrated here
  app.push<segue<ZoomOut>::to<MainMenuScene>>();
  // app.push<MainMenuScene>(); // uncomment this and comment the line above for old behavior

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
      else if (event.type == sf::Event::KeyPressed) {
        // Toggle to different renderers using F-keys
        sf::Keyboard::Key code = event.key.code;
        if (code == sf::Keyboard::F1) {
          app.setRenderer(0);
          window.setTitle("Swoosh Demo (renderer=" + app.getCurrentRendererName() + ")");
        }
        else if (code == sf::Keyboard::F2) {
          app.setRenderer(1);
          window.setTitle("Swoosh Demo (renderer=" + app.getCurrentRendererName() + ")");
        }
      }
    }

    // do not update segues when the window is frozen
    if (!pause) {
      app.update(elapsed);
    }

    // We clear after updating so that other items can copy the screen's contents
    window.clear();

    // Track the mouse and create a light source for this pass on the mouse!
    sf::Vector2f mousepos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    cursor.setPosition(mousepos);

    // draw() will directly draw onto the window's render buffer
    app.draw();

    // Draw the mouse cursor over everything else
    window.draw(cursor);

    // Display to our screen
    window.display();

    elapsed = static_cast<float>(clock.getElapsedTime().asSeconds());
  }

  delete cursorTexture;

  return 0;
}
