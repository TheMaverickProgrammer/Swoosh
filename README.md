# Swoosh
SFML Activity and Segue Mini Library

# Technology
SFML 2.5
C++14

## Optional
Includes visual studio project but will work on other operating systems as long as it has C++14 support

# Video
[Click to watch!](https://streamable.com/r1spz)

# Philosophy 
When creating polished applications it should not be a concern to the user to handle the memory for a scene or video game level. 
These activities are just shells around the incoming or outgoing data in visual form; a container for the important stuff that shows up 
on the target device's screen. The biggest goal when designing this software was allowing user's to write complex transitions as simple as possible 
and have syntax to perform that action be human readble.

# Syntax
Swoosh addresses these issues by wrapping push and pop calls with templated types that expect either a class derived from `Activity` for screens or `Segue` for transition effects.

For example

```
ActivityController controller;
controller.Push<MainMenuScene>();

...

// User selects settings
controller.Push<ActivityController::Segue<SlideInLeft>::To<AppSettingsScene>();
```

The syntax is human readable and flows naturally. Swoosh hides the intricacies from the user so they can focus on what's really important: writing the application!

## Changing Time
The `Segue` class takes in two arguments: the next activity type, and the duration for the transition to last. By default the transition is set to 1 second. 
This may be too fast or too slow for your needs. The `Duration` class takes two types: the SFML function for time like `sf::seconds` and the amount you want.

For example

```
controller.Push<ActivityController::Segue<FadeIn, Duration<&sf::seconds, 5>>::To<DramaticIntroScene>();
```

## Supplying Additional Arguments
Your activity classes may be dependant on external information like loading your game from a save file or displaying important business data exported from another screen. 

```
SaveInfo = info = LoadSaveFile(selectedProfile);
controller.Push<SuperJumpManLevel1>({info.getLives(), info.getCoins(), info.getMapData()});
```

This is the same for segues

```
FinancialInfo* data = loadFinancialResult(calender.getDate());
controller.Push<ActivityController::Segue<CheckerboardEffect, Duration<&sf::seconds, 3>>::To<FinancialReport>(data);
```

# Leaving Activities
The `ActivityController` class can _push_ and _pop_ states but only when it's safe to do so. It does not pop in the middle of a cycle and does not push when in the middle of a segue.
Make sure your activity controller calls are in an Activity's `OnUpdate(double elapsed)` function to avoid having _push_ or _pop_ intents discarded.

## Push
```
controller.Push<MyScene>();
controller.Push<ActivityController::Segue<FadeIn>::To<MyScene>>();
```

## Pop
Pushed activities are added to the stack immediately. However there are steps involved in the controller's update loop that do not make this
safe to do for _pop_. Instead, the function `QueuePop()` is supplied, signalling the controller to pop as soon as it can.

```
controller.QueuePop(); 
controller.QueuePop<ActivityController::Segue<SlideIn>>();
```

# Writing Activities
An activity has 7 states it can be in:
* Starting for the first time
* Entering the focus of the app
* Leaving the focus of the app
* Inactive (but not terminating)
* Resuming 
* Ending (to be terminated)
* Updating 

Here is an example of the most simplest scene using Swoosh:

```
class DemoScene : public Activity {
private:
  sf::Texture* bgTexture;
  sf::Sprite bg;

  sf::Font   menuFont;
  sf::Text   menuText;

public:
  DemoScene(ActivityController& controller) : Activity(controller) { 
    bgTexture = LoadTexture("resources/scene.png");
    bg = sf::Sprite(*bgTexture);

    menuFont.loadFromFile("resources/commando.ttf");
    menuText.setFont(menuFont);

    menuText.setFillColor(sf::Color::Red); 
  }

  virtual void OnStart() {
    std::cout << "DemoScene OnStart called" << std::endl;
  }

  virtual void OnUpdate(double elapsed) {
  }

  virtual void OnLeave() {
    std::cout << "DemoScene OnLeave called" << std::endl;

  }

  virtual void OnExit() {
    std::cout << "DemoScene OnExit called" << std::endl;
  }

  virtual void OnEnter() {
    std::cout << "DemoScene OnEnter called" << std::endl;
  }

  virtual void OnResume() {
    std::cout << "DemoScene OnResume called" << std::endl;

  }

  virtual void OnDraw(sf::RenderTexture& surface) {
    surface.draw(bg);

    menuText.setPosition(sf::Vector2f(200, 100));
    menuText.setString("Hello World");
    surface.draw(menuText);
  }

  virtual void OnEnd() {
    std::cout << "DemoScene OnEnd called" << std::endl;
  }

  virtual ~DemoScene() { delete bgTexture;; }
};
```

# Writing Segues
When writing transitions or action-dependant software, one of the worst things that can happen is to have a buggy action. 
If one action depends on another to finish, but never does, the app will hang in limbo. 

This fact inspired Swoosh to be dependant on a timer. When the timer is up the Segue will be deleted and the next scene 
added on top of the stack. The time elapsed and total time alloted can be retrieved in the class body to make some cool effects
from start to finish.

This example Segue will slide a new screen in while pushing the last scene out. Really cool!

```
#pragma once
#include "Segue.h"
#include "EaseFunctions.h"

class SlideIn : public Segue {
private:
  sf::Texture* temp;
  int direction = 0;

public:
  virtual void OnDraw(sf::RenderTexture& surface) {
    double elapsed = getElapsed().asMilliseconds();
    double duration = getDuration().asMilliseconds();
    double alpha = ease::linear(elapsed, duration, 1);

    this->DrawLastActivity(surface);

    surface.display(); // flip and ready the buffer

    if (temp) delete temp;
    temp = new sf::Texture(surface.getTexture()); // Make a copy of the source texture

    sf::Sprite left(*temp); 

    int lr = 0;
    int ud = 0;

    if (direction == 0) lr = -1;
    if (direction == 1) ud = -1;
    if (direction == 2) lr = 1;
    if (direction == 3) ud = 1;

    left.setPosition(lr * alpha * left.getTexture()->getSize().x, ud * alpha * left.getTexture()->getSize().y);

    surface.clear();

    this->DrawNextActivity(surface);

    surface.display(); // flip and ready the buffer
    sf::Sprite right(surface.getTexture());

    right.setPosition(-lr * (1-alpha) * right.getTexture()->getSize().x, -ud * (1-alpha) * right.getTexture()->getSize().y);

    controller.getWindow().draw(left);
    controller.getWindow().draw(right);

    surface.clear(sf::Color::Transparent);
  }

  SlideIn(sf::Time duration, Activity* last, Activity* next) : Segue(duration, last, next) { 
    /* ... */ 
    temp = nullptr;
    direction = rand() % 4; // Choose a random direction
  }

  virtual ~SlideIn() { ; }
};
```

# Integrating Swoosh into your SFML application
